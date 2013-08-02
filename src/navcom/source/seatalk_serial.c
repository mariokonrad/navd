#include <navcom/source/seatalk_serial.h>
#include <navcom/source/seatalk_serial_private.h>
#include <navcom/property_serial.h>
#include <navcom/property_read.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <device/simulator_serial_seatalk.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/signalfd.h>

enum { STATE_READ, STATE_ESCAPE, STATE_PARITY };

struct seatalk_context_t
{
	int state;
	uint8_t index;
	uint8_t remaining;
	uint8_t data[SEATALK_MAX_SENTENCE];

	uint8_t raw;
	uint32_t collisions;

	struct message_t msg;
};

const char * state_name(int state) /* TODO: used temporarily*/
{
	switch (state) {
		case STATE_READ:   return "READ";
		case STATE_ESCAPE: return "ESCAPE";
		case STATE_PARITY: return "PARITY";
		default: break;
	}
	return "<unknown>";
}

static void dump( /* TODO: used temporarily*/
		struct seatalk_context_t * ctx,
		uint8_t c,
		const char * type)
{
	printf("%-6s : %-4s : %3u : 0x%02x\n", state_name(ctx->state), type, ctx->remaining, c);
}

static void dump_sentence(struct seatalk_context_t * ctx) /* TODO: used temporarily*/
{
	const char * title;

	if (ctx->remaining)
		return;

	switch (ctx->data[0]) {
		case 0x00: title = "depth"; break;
		case 0x10: title = "apparent wind angle"; break;
		case 0x11: title = "apparent wind speed"; break;
		case 0x20: title = "speed through water"; break;
		case 0x23: title = "water temperature C/F"; break;
		case 0x26: title = "speed through water (detailed)"; break;
		case 0x27: title = "water temperature C"; break;
		default:   title = "unknown"; break;
	}

	printf("========================== sentence: %s\n", title);
}

static void seatalk_context_init(struct seatalk_context_t * ctx)
{
	memset(ctx, 0, sizeof(*ctx));

	ctx->state = STATE_READ;
	ctx->remaining = 255;
	ctx->index = 0;
}

static uint8_t parity(uint8_t a)
{
	int i;
	int c = 0;

	for (i = 0; i < 8; ++i) {
		if (a & 0x01)
			++c;
		a >>= 1;
	}
	return (c % 2) == 0;
}

static void seatalk_context_write_cmd(
		struct seatalk_context_t * ctx,
		uint8_t c)
{
	if (ctx->remaining > 0 && ctx->remaining < 254) {
		++ctx->collisions;
		syslog(LOG_NOTICE, "SeaTalk bus collision (%u)", ctx->collisions);
	}

	dump(ctx, c, "cmd");

	ctx->data[0] = c;
	ctx->index = 1;
	ctx->remaining = 254;
}

/**
 * Writes data into the read context buffer.
 */
static void seatalk_context_write_data(
		struct seatalk_context_t * ctx,
		uint8_t c)
{
	dump(ctx, c, "data");

	if (ctx->index >= sizeof(ctx->data))
		return;

	if (ctx->remaining == 0)
		return;

	if (ctx->remaining == 255) /* not yet in sync */
		return;

	if (ctx->remaining == 254) {
		/* attribute byte, -1 because cmd is already consumed */
		ctx->remaining = 3 + (c & 0x0f) - 1;
	}

	ctx->data[ctx->index] = c;
	++ctx->index;
	--ctx->remaining;
}

/**
 * Sends a message containing the read SeaTalk sentence.
 *
 * @param[in] config Process configuration
 * @param[out] buf Working context.
 * @retval EXIT_SUCCESS Success (program working correctly, maybe wrong data)
 * @retval EXIT_FAILURE Failure
 */
static int emit_message(
		const struct proc_config_t * config,
		struct seatalk_context_t * ctx)
{
	int rc;

	dump_sentence(ctx);

	ctx->msg.type = MSG_SEATALK;
	rc = seatalk_read(&ctx->msg.data.attr.seatalk, ctx->data, ctx->index);
	if (rc == 0) {
		rc = message_write(config->wfd, &ctx->msg);
		if (rc != EXIT_SUCCESS)
			syslog(LOG_ERR, "unable to write SeaTalk data: %s", strerror(errno));
	} else if (rc == -4) {
		syslog(LOG_INFO, "unknown seatalk sentence, discarding");
		return EXIT_SUCCESS;
	} else {
		syslog(LOG_ERR, "error: seatalk_read, rc=%d", rc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Processes SeaTalk data read from the device.
 *
 * This function contains a state machine, which does the handling
 * of the SeaTalk specific feature: misusing the parity bit as
 * indicator for command bytes.
 * Since termios is in use, which provides parity error information
 * as quoting bytes, a non-trivial implementation is needed to
 * distinguish between normal and command bytes. Also, collision
 * detection on this pseudo-bus (SeaTalk) is handled.
 *
 * @param[in] config Process configuration
 * @param[out] buf Working context.
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int process_seatalk(
		const struct proc_config_t * config,
		struct seatalk_context_t * ctx)
{
	switch (ctx->state) {
		case STATE_READ:
			if (ctx->raw == 0xff) {
				ctx->state = STATE_ESCAPE;
			} else {
				if (parity(ctx->raw)) {
					seatalk_context_write_cmd(ctx, ctx->raw);
				} else {
					seatalk_context_write_data(ctx, ctx->raw);
					if (ctx->remaining == 0)
						if (emit_message(config, ctx) != EXIT_SUCCESS)
							return EXIT_FAILURE;
				}
			}
			break;

		case STATE_ESCAPE:
			if (ctx->raw == 0x00) {
				ctx->state = STATE_PARITY;
			} else if (ctx->raw == 0xff) {
				seatalk_context_write_data(ctx, ctx->raw);
				if (ctx->remaining == 0)
					if (emit_message(config, ctx) != EXIT_SUCCESS)
						return EXIT_FAILURE;
				ctx->state = STATE_READ;
			} else {
				dump(ctx, ctx->raw, "ERR");
				return EXIT_FAILURE;
			}
			break;

		case STATE_PARITY:
			if (parity(ctx->raw)) {
				seatalk_context_write_data(ctx, ctx->raw);
				if (ctx->remaining == 0)
					if (emit_message(config, ctx) != EXIT_SUCCESS)
						return EXIT_FAILURE;
			} else {
				seatalk_context_write_cmd(ctx, ctx->raw);
			}
			ctx->state = STATE_READ;
			break;
	}

	return EXIT_SUCCESS;
}

/**
 * Reads data from the device.
 *
 * @param[in] ops Device operations
 * @param[in] device Device to operate on
 * @param[out] data Working context.
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int read_data(
		const struct device_operations_t * ops,
		struct device_t * device,
		struct seatalk_context_t * buf)
{
	int rc;

	rc = ops->read(device, (char *)&buf->raw, sizeof(buf->raw));
	if (rc < 0) {
		syslog(LOG_ERR, "unable to read from device: %s", strerror(errno));
		return EXIT_FAILURE;
	}
	if (rc != sizeof(buf->raw)) {
		syslog(LOG_ERR, "invalid size read from device: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Initializes the configuration data with default values.
 *
 * @param[out] data The configuration to initialize.
 */
static void init_data(struct seatalk_serial_data_t * data)
{
	memset(data, 0, sizeof(struct seatalk_serial_data_t));

	/* device type */
	strncpy(data->type, "serial", sizeof(data->type));

	/* default values for the default device type */
	strncpy(data->config.serial.name, "/dev/ttyUSB0", sizeof(data->config.serial.name));
	data->config.serial.baud_rate = BAUD_4800;
	data->config.serial.data_bits = DATA_BIT_8;
	data->config.serial.stop_bits = STOP_BIT_1;
	data->config.serial.parity = PARITY_MARK;
}

/**
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	struct seatalk_serial_data_t * data = NULL;

	if (config == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct seatalk_serial_data_t *)malloc(sizeof(struct seatalk_serial_data_t));
	config->data = data;
	init_data(data);

	/* device type */
	if (property_read_string(properties, "_devicetype_", data->type, sizeof(data->type)) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	/* device properties */
	if (prop_serial_read_device(&data->config.serial, properties, "device") != EXIT_SUCCESS)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

/**
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int exit_proc(struct proc_config_t * config)
{
	if (config == NULL)
		return EXIT_FAILURE;

	if (config->data) {
		free(config->data);
		config->data = NULL;
	}

	return EXIT_SUCCESS;
}

/**
 * Determines the device configuration and operation according to
 * the specified configuration (containing the device type).
 *
 * @param[in] data The source specific data.
 * @param[out] ops The device operations, NULL if no valid device type
 *   is configured.
 * @param[out] device_config The device configuration, NULL if no valid
 *   device type is configured.
 */
static void get_device_ops(
		const struct seatalk_serial_data_t * data,
		const struct device_operations_t ** ops,
		const struct device_config_t ** device_config)
{
	*ops = NULL;
	*device_config = NULL;

	if (strcmp(data->type, "serial") == 0) {
		*ops = &serial_device_operations;
		*device_config = (const struct device_config_t *)&data->config.serial;
		return;
	}
	if (strcmp(data->type, "simulator_serial_seatalk") == 0) {
		*ops = &simulator_serial_seatalk_operations;
		return;
	}
}

static int proc(struct proc_config_t * config)
{
	int rc;

	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timeval tm;
	struct signalfd_siginfo signal_info;

	struct device_t device;
	const struct device_operations_t * ops = NULL;
	const struct device_config_t * device_config = NULL;

	struct seatalk_context_t readbuf;
	struct seatalk_serial_data_t * data;

	if (!config)
		return EXIT_FAILURE;

	data = (struct seatalk_serial_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	get_device_ops(data, &ops, &device_config);
	if (!ops) {
		syslog(LOG_ERR, "unknown device type: '%s'", data->type);
		return EXIT_FAILURE;
	}

	seatalk_context_init(&readbuf);

	device_init(&device);
	rc = ops->open(&device, device_config);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to open device");
		return EXIT_FAILURE;
	}

	while (1) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max)
			fd_max = config->rfd;
		FD_SET(device.fd, &rfds);
		if (device.fd > fd_max)
			fd_max = device.fd;
		FD_SET(config->signal_fd, &rfds);
		if (config->signal_fd > fd_max)
			fd_max = config->signal_fd;

		tm.tv_sec = 1;
		tm.tv_usec = 0;

		rc = select(fd_max + 1, &rfds, NULL, NULL, &tm);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			continue;
		}

		if (FD_ISSET(config->signal_fd, &rfds)) {
			rc = read(config->signal_fd, &signal_info, sizeof(signal_info));
			if (rc < 0 || rc != sizeof(signal_info)) {
				syslog(LOG_ERR, "cannot read singal info");
				return EXIT_FAILURE;
			}

			if (signal_info.ssi_signo == SIGTERM)
				break;
		}

		if (FD_ISSET(device.fd, &rfds)) {
			if (read_data(ops, &device, &readbuf) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			if (process_seatalk(config, &readbuf) != EXIT_SUCCESS)
				return EXIT_FAILURE;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			if (message_read(config->rfd, &msg) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.attr.system) {
						case SYSTEM_TERMINATE:
							rc = ops->close(&device);
							if (rc < 0) {
								syslog(LOG_ERR, "unable to close device: %s", strerror(errno));
								return EXIT_FAILURE;
							}
							return EXIT_SUCCESS;
						default:
							break;
					}
				default:
					break;
			}
			continue;
		}
	}
	return EXIT_SUCCESS;
}

static void help(void)
{
	printf("\n");
	printf("seatalk_serial\n");
	printf("\n");
	printf("Receives navigational data on a serial interface and\n");
	printf("sends messages accoringly.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  device : the device to read data from\n");
	printf("\n");
	printf("  _devicetype_ : testing only\n");
	printf("\n");
	printf("Example:\n");
	printf("  seatalk : seatalk_serial { device:'/dev/ttyUSB0' };\n");
	printf("\n");
}

const struct proc_desc_t seatalk_serial = {
	.name = "seatalk_serial",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = help,
};

