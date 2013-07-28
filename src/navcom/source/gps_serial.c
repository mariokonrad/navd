#include <navcom/source/gps_serial.h>
#include <navcom/source/gps_serial_private.h>
#include <navcom/property_serial.h>
#include <navcom/property_read.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <device/simulator_serial_gps.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/signalfd.h>

struct read_buffer_t
{
	int index;
	char raw;
	char data[NMEA_MAX_SENTENCE + 1];
	struct message_t msg;
};

/**
 * Initializes the configuration data with default values.
 *
 * @param[out] data The configuration to initialize.
 */
static void init_default_data(struct gps_serial_data_t * data)
{
	memset(data, 0, sizeof(*data));

	/* device type */
	strncpy(data->type, "serial", sizeof(data->type));

	/* default values for the default device type */
	strncpy(data->config.serial.name, "/dev/ttyUSB0", sizeof(data->config.serial.name));
	data->config.serial.baud_rate = BAUD_4800;
	data->config.serial.data_bits = DATA_BIT_8;
	data->config.serial.stop_bits = STOP_BIT_1;
	data->config.serial.parity = PARITY_NONE;
}

/**
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	struct gps_serial_data_t * data = NULL;

	if (config == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct gps_serial_data_t *)malloc(sizeof(struct gps_serial_data_t));
	init_default_data(data);
	config->data = data;

	/* device type */
	if (property_read_string(properties, "_devicetype_", data->type, sizeof(data->type)) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	/* device properties */
	if (prop_serial_read_device(&data->config.serial, properties, "device") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_baudrate(&data->config.serial, properties, "baud") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_parity(&data->config.serial, properties, "parity") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_databits(&data->config.serial, properties, "data") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_stopbits(&data->config.serial, properties, "stop") != EXIT_SUCCESS)
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
 * Processes NMEA data read from the device.
 *
 * @param[in] config Process configuration
 * @param[out] buf Working context.
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int process_nmea(
		const struct proc_config_t * config,
		struct read_buffer_t * buf)
{
	int rc;

	nmea_init(&buf->msg.data.nmea);
	switch (buf->raw) {
		case '\r':
			break;
		case '\n':
			rc = nmea_read(&buf->msg.data.nmea, buf->data);
			if (rc == 0) {
				rc = message_write(config->wfd, &buf->msg);
				if (rc != EXIT_SUCCESS)
					syslog(LOG_ERR, "unable to write NMEA data: %s", strerror(errno));
			} else if (rc == 1) {
				syslog(LOG_ERR, "unknown sentence: '%s'", buf->data);
			} else if (rc == -2) {
				syslog(LOG_ERR, "checksum error: '%s'", buf->data);
			} else {
				syslog(LOG_ERR, "parameter error");
				return EXIT_FAILURE;
			}
			buf->index = 0;
			buf->data[buf->index] = 0;
			break;
		default:
			if (buf->index < NMEA_MAX_SENTENCE) {
				buf->data[buf->index++] = buf->raw;
			} else {
				syslog(LOG_ERR, "sentence too long, discarding");
				buf->index = 0;
			}
			buf->data[buf->index] = 0;
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
		struct read_buffer_t * buf)
{
	int rc;

	rc = ops->read(device, &buf->raw, sizeof(buf->raw));
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
		const struct gps_serial_data_t * data,
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
	if (strcmp(data->type, "simulator_serial_gps") == 0) {
		*ops = &simulator_serial_gps_operations;
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

	struct read_buffer_t readbuf;
	struct gps_serial_data_t * data;

	if (!config)
		return EXIT_FAILURE;

	data = (struct gps_serial_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	get_device_ops(data, &ops, &device_config);
	if (!ops) {
		syslog(LOG_ERR, "unknown device type: '%s'", data->type);
		return EXIT_FAILURE;
	}

	memset(&readbuf, 0, sizeof(readbuf));
	readbuf.msg.type = MSG_NMEA;

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
			if (process_nmea(config, &readbuf) != EXIT_SUCCESS)
				return EXIT_FAILURE;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			if (message_read(config->rfd, &msg) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
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
	printf("gps_serial\n");
	printf("\n");
	printf("Receives navigational data on a serial interface and\n");
	printf("sends messages accoringly.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  device : the device to read data from\n");
	printf("  baud   : baud rate to operate on, valid values:\n");
	printf("           300, 600, 1200, 2400, 4800, 9600, 19200\n");
	printf("           34800, 57600, 115200, 230400\n");
	printf("  parity : parity check to use, valid values:\n");
	printf("           none, even, odd, mark\n");
	printf("  data   : number of data bits, valid values:\n");
	printf("           5, 6, 7, 8\n");
	printf("  stop   : number of stop bits, valid values:\n");
	printf("           1, 2\n");
	printf("\n");
	printf("  _devicetype_ : testing only\n");
	printf("\n");
	printf("Example:\n");
	printf("  gps : gps_serial { device:'/dev/ttyUSB0', baud:4800, parity:'none', data:8, stop:1 };\n");
	printf("\n");
}

const struct proc_desc_t gps_serial = {
	.name = "gps_serial",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = help,
};

