#include <navcom/destination/nmea_serial.h>
#include <navcom/property_serial.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

static int initialized = 0;
static struct serial_config_t serial_config = {
	"/dev/ttyUSB1",
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
	struct message_t msg;

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				return EXIT_FAILURE;
			}
			if (rc != (int)sizeof(msg) || rc == 0) {
				syslog(LOG_ERR, "cannot read message, rc=%d", rc);
				return EXIT_FAILURE;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							return EXIT_SUCCESS;
						default:
							break;
					}
					break;

				case MSG_NMEA:
					/* TODO: send message to device */
					break;

				default:
					syslog(LOG_WARNING, "unknown msg type: %08x\n", msg.type);
					break;
			}
			continue;
		}
	}
	return EXIT_SUCCESS;
}

const struct proc_desc_t nmea_serial ={
	.name = "nmea_serial",
	.configure = configure,
	.func = proc
};

