#include <navcom/source/gps_serial.h>
#include <navcom/property_serial.h>
#include <navcom/message.h>
#include <device/serial.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/select.h>

static int initialized = 0;
static struct serial_config_t serial_config = {
	"/dev/ttyUSB0",
	BAUD_4800,
	DATA_BIT_8,
	STOP_BIT_1,
	PARITY_NONE
};

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	if (prop_serial_read_device(&serial_config, properties, "device") != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_serial_read_baudrate(&serial_config, properties, "baud") != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_serial_read_parity(&serial_config, properties, "parity") != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_serial_read_databits(&serial_config, properties, "data") != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_serial_read_stopbits(&serial_config, properties, "stop") != EXIT_SUCCESS) return EXIT_FAILURE;

	initialized = 1;
	return EXIT_SUCCESS;
}

static int proc(const struct proc_config_t * config)
{
	int rc;

	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	struct device_t device;
	const struct device_operations_t * ops = NULL;

	struct message_t msg_nmea;
	struct nmea_t * nmea = &msg_nmea.data.nmea;
	char buf[NMEA_MAX_SENTENCE + 1];
	int buf_index;
	char c;

	if (!initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	ops = &serial_device_operations;
	device_init(&device);
	memset(buf, 0, sizeof(buf));
	buf_index = 0;

	msg_nmea.type = MSG_NMEA;

	rc = ops->open(&device, (const struct device_config_t *)&serial_config);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to open device '%s'", serial_config.name);
		return EXIT_FAILURE;
	}

	while (!request_terminate) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max) fd_max = config->rfd;
		FD_SET(device.fd, &rfds);
		if (device.fd > fd_max) fd_max = device.fd;

		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (FD_ISSET(device.fd, &rfds)) {
			rc = ops->read(&device, &c, sizeof(c));
			if (rc < 0) {
				syslog(LOG_ERR, "unable to read from device '%s': %s", serial_config.name, strerror(errno));
				return EXIT_FAILURE;
			}
			if (rc != sizeof(c)) {
				syslog(LOG_ERR, "invalid size read from device '%s': %s", serial_config.name, strerror(errno));
				return EXIT_FAILURE;
			}
			nmea_init(nmea);
			switch (c) {
				case '\r':
					break;
				case '\n':
					rc = nmea_read(nmea, buf);
					if (rc == 0) {
						rc = write(config->wfd, &msg_nmea, sizeof(msg_nmea));
						if (rc < 0) {
							syslog(LOG_ERR, "unable to read from pipe: %s", strerror(errno));
						}
					} else if (rc == 1) {
						syslog(LOG_ERR, "unknown sentence: '%s'", buf);
					} else if (rc == -2) {
						syslog(LOG_ERR, "checksum error: '%s'", buf);
					} else {
						syslog(LOG_ERR, "parameter error");
						return EXIT_FAILURE;
					}
					buf_index = 0;
					buf[buf_index] = 0;
					break;
				default:
					if (buf_index < NMEA_MAX_SENTENCE) {
						buf[buf_index++] = c;
					} else {
						syslog(LOG_ERR, "sentence too long, discarding");
						buf_index = 0;
					}
					buf[buf_index] = 0;
					break;
			}
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				syslog(LOG_ERR, "unable to read from pipe: %s", strerror(errno));
				continue;
			}
			if (rc != (int)sizeof(msg) || rc == 0) {
				syslog(LOG_ERR, "cannot read message, rc=%d", rc);
				return EXIT_FAILURE;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							rc = ops->close(&device);
							if (rc < 0) {
								syslog(LOG_ERR, "unable to close device '%s': %s", serial_config.name, strerror(errno));
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
	.configure = configure,
	.func = proc,
	.clean = NULL
};

