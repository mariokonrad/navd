#include <navcom/gps_serial.h>
#include <navcom/message.h>
#include <device/serial.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

static int proc(const struct proc_config_t * config, const struct property_list_t * properties)
{
	int rc;

	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	struct device_t device;
	const struct device_operations_t * ops = NULL;

	struct nmea_t nmea;
	char buf[NMEA_MAX_SENTENCE + 1];
	int buf_index;
	char c;

	/* TODO:configuration */

	struct serial_config_t serial_config = {
		"/dev/ttyUSB0",
		BAUD_4800,
		DATA_BIT_8,
		STOP_BIT_1,
		PARITY_NONE
	};

	UNUSED_ARG(properties);

	ops = &serial_device_operations;
	device_init(&device);
	nmea_init(&nmea);
	memset(buf, 0, sizeof(buf));
	buf_index = 0;

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
			switch (c) {
				case '\r':
					break;
				case '\n':
					rc = nmea_read(&nmea, buf);
					if (rc == 0) {
						printf("OK : [%s]\n", buf);
						/* TODO: send message */
					} else if (rc == 1) {
						printf("[%s] : UNKNOWN SENTENCE\n", buf);
					} else if (rc == -2) {
						printf("[%s] : CHECKSUM ERROR\n", buf);
					} else {
						fprintf(stderr, "parameter error\n");
						return EXIT_FAILURE;
					}
					buf_index = 0;
					buf[buf_index] = 0;
					break;
				default:
					if (buf_index < NMEA_MAX_SENTENCE) {
						buf[buf_index++] = c;
					} else {
						fprintf(stderr, "sentence too long, discarding\n");
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
	"gps_serial_demo",
	NULL,
	proc
};



