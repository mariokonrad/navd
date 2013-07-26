#include <navcom/source/seatalk_serial.h>
#include <navcom/source/seatalk_serial_private.h>
#include <navcom/property_serial.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/select.h>

/* TODO: make is possible to use other devices than 'serial', to support better testing */

enum { STATE_READ, STATE_ESCAPE, STATE_PARITY };

struct seatalk_context_t
{
	int state;
	uint8_t index;
	uint8_t remaining;
	uint8_t data[18];

	struct message_t msg;
};

#if 0 /* TODO: disabled for now  ================================== */
const char * state_name(int state)
{
	switch (state) {
		case STATE_READ:   return "READ";
		case STATE_ESCAPE: return "ESCAPE";
		case STATE_PARITY: return "PARITY";
		default: break;
	}
	return "<unknown>";
}

static void dump(
		struct seatalk_context_t * ctx,
		uint8_t c,
		const char * type)
{
	printf("%-6s : %-4s : %3u : 0x%02x\n", state_name(ctx->state), type, ctx->remaining, c);
}

static void dump_sentence(struct seatalk_context_t * ctx)
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

static void seatalk_context_init(struct seatalk_context_t * ctx)
{
	memset(ctx, 0, sizeof(*ctx));

	ctx->state = STATE_READ;
	ctx->remaining = 255;
	ctx->index = 0;
}

static void seatalk_context_write_cmd(struct seatalk_context_t * ctx, uint8_t c)
{
	if (ctx->remaining > 0 && ctx->remaining < 254) {
		printf("## collision -> restart cmd\n");
	}

	dump(ctx, c, "cmd");

	ctx->data[0] = c;
	ctx->index = 1;
	ctx->remaining = 254;
}

static void seatalk_context_write_data(struct seatalk_context_t * ctx, uint8_t c)
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

static int process(struct seatalk_context_t * ctx)
{
	uint8_t c;

	if (read(&c) != 1)
		return -1;

	switch (ctx->state) {
		case STATE_READ:
			if (c == 0xff) {
				ctx->state = STATE_ESCAPE;
			} else {
				if (parity(c)) {
					if (ctx->remaining > 0 && ctx->remaining < 254) {
						/* collision */
					}
					seatalk_context_write_cmd(ctx, c);
				} else {
					seatalk_context_write_data(ctx, c);
					if (ctx->remaining == 0)
						dump_sentence(ctx);
				}
			}
			break;

		case STATE_ESCAPE:
			if (c == 0x00) {
				ctx->state = STATE_PARITY;
			} else if (c == 0xff) {
				seatalk_context_write_data(ctx, c);
				if (ctx->remaining == 0)
					dump_sentence(ctx);
				ctx->state = STATE_READ;
			} else {
				dump(ctx, c, "ERR");
				return -1;
			}
			break;

		case STATE_PARITY:
			if (parity(c)) {
				seatalk_context_write_data(ctx, c);
				if (ctx->remaining == 0)
					dump_sentence(ctx);
			} else {
				if (ctx->remaining > 0) {
					/* collision */
				}
				seatalk_context_write_cmd(ctx, c);
			}
			ctx->state = STATE_READ;
			break;
	}

	return 0;
}
#endif /* ================================================================ */

/**
 * Initializes the configuration data with default values.
 *
 * @param[out] data The configuration to initialize.
 */
static void init_data(struct seatalk_serial_data_t * data)
{
	memset(data, 0, sizeof(struct seatalk_serial_data_t));

	/* TODO: initialize seatalk read context */

	strncpy(data->serial_config.name, "/dev/ttyUSB0", sizeof(data->serial_config.name));
	data->serial_config.baud_rate = BAUD_4800;
	data->serial_config.data_bits = DATA_BIT_8;
	data->serial_config.stop_bits = STOP_BIT_1;
	data->serial_config.parity = PARITY_MARK;
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

	if (prop_serial_read_device(&data->serial_config, properties, "device") != EXIT_SUCCESS)
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

static int proc(struct proc_config_t * config)
{
	int rc;

	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	struct device_t device;
	const struct device_operations_t * ops = NULL;

	struct seatalk_context_t readbuf;
	struct seatalk_serial_data_t * data;

	if (!config)
		return EXIT_FAILURE;

	data = (struct seatalk_serial_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	ops = &serial_device_operations;
	device_init(&device);
	memset(&readbuf, 0, sizeof(readbuf));
	readbuf.msg.type = MSG_SEATALK;

	rc = ops->open(&device, (const struct device_config_t *)&data->serial_config);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to open device '%s'", data->serial_config.name);
		return EXIT_FAILURE;
	}

	while (!proc_request_terminate()) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max) fd_max = config->rfd;
		FD_SET(device.fd, &rfds);
		if (device.fd > fd_max) fd_max = device.fd;

		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

/* TODO: disabled processing of acutal data
		if (FD_ISSET(device.fd, &rfds))
			if (process_nmea(config, ops, &device, &readbuf) != EXIT_SUCCESS)
				return EXIT_FAILURE;
*/

		if (FD_ISSET(config->rfd, &rfds)) {
			if (message_read(config->rfd, &msg) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							rc = ops->close(&device);
							if (rc < 0) {
								syslog(LOG_ERR, "unable to close device '%s': %s",
									data->serial_config.name, strerror(errno));
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

const struct proc_desc_t seatalk_serial = {
	.name = "seatalk_serial",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = NULL,
};

