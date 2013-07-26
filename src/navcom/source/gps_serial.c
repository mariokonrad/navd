#include <navcom/source/gps_serial.h>
#include <navcom/source/gps_serial_private.h>
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

struct nmea_read_buffer_t
{
	int index;
	char data[NMEA_MAX_SENTENCE + 1];
	struct message_t msg;
};

/**
 * Initializes the configuration data with default values.
 *
 * @param[out] data The configuration to initialize.
 */
static void init_data(struct gps_serial_data_t * data)
{
	memset(data, 0, sizeof(struct gps_serial_data_t));

	strncpy(data->serial_config.name, "/dev/ttyUSB0", sizeof(data->serial_config.name));
	data->serial_config.baud_rate = BAUD_4800;
	data->serial_config.data_bits = DATA_BIT_8;
	data->serial_config.stop_bits = STOP_BIT_1;
	data->serial_config.parity = PARITY_NONE;
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
	config->data = data;
	init_data(data);

	if (prop_serial_read_device(&data->serial_config, properties, "device") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_baudrate(&data->serial_config, properties, "baud") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_parity(&data->serial_config, properties, "parity") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_databits(&data->serial_config, properties, "data") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_stopbits(&data->serial_config, properties, "stop") != EXIT_SUCCESS)
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
 * Reads data from the device and processes them as NMEA data.
 *
 * @param[in] config
 * @param[in] ops Device operations
 * @param[in] device Device to operate on
 * @param[out] buf Working context
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int process_nmea(
		struct proc_config_t * config,
		const struct device_operations_t * ops,
		struct device_t * device,
		struct nmea_read_buffer_t * buf)
{
	char c;
	int rc;
	struct gps_serial_data_t * data = (struct gps_serial_data_t *)config->data;

	rc = ops->read(device, &c, sizeof(c));
	if (rc < 0) {
		syslog(LOG_ERR, "unable to read from device '%s': %s", data->serial_config.name, strerror(errno));
		return EXIT_FAILURE;
	}
	if (rc != sizeof(c)) {
		syslog(LOG_ERR, "invalid size read from device '%s': %s", data->serial_config.name, strerror(errno));
		return EXIT_FAILURE;
	}
	nmea_init(&buf->msg.data.nmea);
	switch (c) {
		case '\r':
			break;
		case '\n':
			rc = nmea_read(&buf->msg.data.nmea, buf->data);
			if (rc == 0) {
				rc = message_write(config->wfd, &buf->msg);
				if (rc != EXIT_SUCCESS) {
					syslog(LOG_ERR, "unable to write NMEA data: %s", strerror(errno));
				}
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
				buf->data[buf->index++] = c;
			} else {
				syslog(LOG_ERR, "sentence too long, discarding");
				buf->index = 0;
			}
			buf->data[buf->index] = 0;
			break;
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

	struct nmea_read_buffer_t readbuf;
	struct gps_serial_data_t * data;

	if (!config)
		return EXIT_FAILURE;

	data = (struct gps_serial_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	ops = &serial_device_operations;
	device_init(&device);
	memset(&readbuf, 0, sizeof(readbuf));
	readbuf.msg.type = MSG_NMEA;

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

		if (FD_ISSET(device.fd, &rfds))
			if (process_nmea(config, ops, &device, &readbuf) != EXIT_SUCCESS)
				return EXIT_FAILURE;

		if (FD_ISSET(config->rfd, &rfds)) {
			if (message_read(config->rfd, &msg) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							rc = ops->close(&device);
							if (rc < 0) {
								syslog(LOG_ERR, "unable to close device '%s': %s", data->serial_config.name, strerror(errno));
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

const struct proc_desc_t gps_serial = {
	.name = "gps_serial",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = NULL,
};

