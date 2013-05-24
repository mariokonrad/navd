#include <navcom/destination/nmea_serial.h>
#include <navcom/property_serial.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

/* TODO: static data must be part of config */
static int initialized = 0;
static struct serial_config_t serial_config = {
	"/dev/ttyUSB1",
	BAUD_4800,
	DATA_BIT_8,
	STOP_BIT_1,
	PARITY_NONE
};

static int send_data(
		const struct device_operations_t * ops,
		struct device_t * device,
		const struct message_t * msg)
{
	int rc;
	char buf[NMEA_MAX_SENTENCE + 1];

	memset(buf, 0, sizeof(buf));
	rc = nmea_write(buf, sizeof(buf), &msg->data.nmea);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to write NMEA data to buffer");
		return EXIT_FAILURE;
	}

	rc = ops->write(device, buf, (uint32_t)rc);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to write to serial device: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	if (!config)
		return EXIT_FAILURE;
	if (!properties)
		return EXIT_FAILURE;

	if (prop_serial_read_device(&serial_config, properties, "device") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_baudrate(&serial_config, properties, "baud") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_parity(&serial_config, properties, "parity") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_databits(&serial_config, properties, "data") != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (prop_serial_read_stopbits(&serial_config, properties, "stop") != EXIT_SUCCESS)
		return EXIT_FAILURE;

	initialized = 1;
	return EXIT_SUCCESS;
}

static int exit_proc(struct proc_config_t * config)
{
	UNUSED_ARG(config);
	return EXIT_SUCCESS;
}

static int proc(struct proc_config_t * config)
{
	int rc;
	fd_set rfds;
	struct message_t msg;

	struct device_t device;
	const struct device_operations_t * ops = NULL;

	if (!initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	ops = &serial_device_operations;
	device_init(&device);

	rc = ops->open(&device, (const struct device_config_t *)&serial_config);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to open device '%s'", serial_config.name);
		return EXIT_FAILURE;
	}

	while (!proc_request_terminate()) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			if (message_read(config->rfd, &msg) != EXIT_SUCCESS)
				return EXIT_FAILURE;
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
					send_data(ops, &device, &msg);
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
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
};

