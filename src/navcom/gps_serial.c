#include <navcom/gps_serial.h>
#include <navcom/message.h>
#include <device/serial.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

static int initialized = 0;
static struct serial_config_t serial_config = {
	"/dev/ttyUSB0",
	BAUD_4800,
	DATA_BIT_8,
	STOP_BIT_1,
	PARITY_NONE
};

static int prop_device(const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;

	prop = proplist_find(properties, "device");
	if (!prop) {
		syslog(LOG_DEBUG, "no device configured, using default '%s'", serial_config.name);
		return EXIT_SUCCESS;
	}

	strncpy(serial_config.name, prop->value, sizeof(serial_config.name));
	syslog(LOG_DEBUG, "device configured, using '%s'", serial_config.name);
	return EXIT_SUCCESS;
}

static int prop_baud_rate(const struct property_list_t * properties)
{
	Baud baud;
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "baud");
	if (!prop) {
		syslog(LOG_DEBUG, "no baud rate configured, using default '%u'", serial_config.baud_rate);
		return EXIT_SUCCESS;
	}

	baud = (Baud)strtoul(prop->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value for property '%s': '%s'", prop->key, prop->value);
		return EXIT_FAILURE;
	}

	switch (baud) {
		case BAUD_300:
		case BAUD_600:
		case BAUD_1200:
		case BAUD_2400:
		case BAUD_4800:
		case BAUD_9600:
		case BAUD_19200:
		case BAUD_38400:
		case BAUD_57600:
		case BAUD_115200:
			serial_config.baud_rate = baud;
			syslog(LOG_DEBUG, "baud rate configured, using '%u'", serial_config.baud_rate);
			break;
		default:
			syslog(LOG_ERR, "invalid baud rate configured: %u", baud);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int prop_parity(const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;

	prop = proplist_find(properties, "parity");
	if (!prop) {
		syslog(LOG_DEBUG, "no parity configured, using default '%d'", serial_config.parity);
		return EXIT_SUCCESS;
	}

	if (strcmp(prop->value, "none") == 0) {
		serial_config.parity = PARITY_NONE;
	} else if (strcmp(prop->value, "even") == 0) {
		serial_config.parity = PARITY_EVEN;
	} else if (strcmp(prop->value, "odd") == 0) {
		serial_config.parity = PARITY_ODD;
	} else {
		syslog(LOG_ERR, "invalid parity configured: %s", prop->value);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int prop_data_bits(const struct property_list_t * properties)
{
	DataBits bits;
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "data");
	if (!prop) {
		syslog(LOG_DEBUG, "no data bits configured, using default '%d'", serial_config.data_bits);
		return EXIT_SUCCESS;
	}

	bits = (DataBits)strtoul(prop->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value for property '%s': '%s'", prop->key, prop->value);
		return EXIT_FAILURE;
	}

	switch (bits) {
		case DATA_BIT_7:
		case DATA_BIT_8:
			serial_config.data_bits = bits;
			syslog(LOG_DEBUG, "data bits configured, using '%u'", serial_config.data_bits);
		default:
			syslog(LOG_ERR, "invalid data bits configured: %u", bits);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int prop_stop_bits(const struct property_list_t * properties)
{
	StopBits bits;
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "stop");
	if (!prop) {
		syslog(LOG_DEBUG, "no stop bits configured, using default '%d'", serial_config.stop_bits);
		return EXIT_SUCCESS;
	}

	bits = (DataBits)strtoul(prop->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value for property '%s': '%s'", prop->key, prop->value);
		return EXIT_FAILURE;
	}

	switch (bits) {
		case STOP_BIT_1:
		case STOP_BIT_2:
			serial_config.stop_bits = bits;
			syslog(LOG_DEBUG, "stop configured, using '%u'", serial_config.stop_bits);
		default:
			syslog(LOG_ERR, "invalid stop bits configured: %u", bits);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int prop(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	if (prop_device(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_baud_rate(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_parity(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_data_bits(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (prop_stop_bits(properties) != EXIT_SUCCESS) return EXIT_FAILURE;

	initialized = 1;
	return EXIT_SUCCESS;
}

static int proc(const struct proc_config_t * config, const struct property_list_t * properties)
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

	UNUSED_ARG(properties);

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
		perror("open");
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
			perror("select");
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (FD_ISSET(device.fd, &rfds)) {
			rc = ops->read(&device, &c, sizeof(c));
			if (rc < 0) {
				perror("read");
				return EXIT_FAILURE;
			}
			if (rc != sizeof(c)) {
				perror("read");
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
							perror("write");
							syslog(LOG_DEBUG, "wfd=%d", config->wfd);
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
				perror("read");
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
								perror("close");
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
	"gps_serial",
	prop,
	proc
};

