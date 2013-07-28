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
#include <sys/signalfd.h>

/**
 * Source specific data.
 */
struct nmea_serial_data_t
{
	int initialized;
	struct serial_config_t serial_config;
};

static void init_data(struct nmea_serial_data_t * data)
{
	memset(data, 0, sizeof(struct nmea_serial_data_t));

	strncpy(data->serial_config.name, "/dev/ttyUSB1", sizeof(data->serial_config));
	data->serial_config.baud_rate = BAUD_4800;
	data->serial_config.data_bits = DATA_BIT_8;
	data->serial_config.stop_bits = STOP_BIT_1;
	data->serial_config.parity = PARITY_NONE;
}

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
	struct nmea_serial_data_t * data = NULL;

	if (!config)
		return EXIT_FAILURE;
	if (!properties)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct nmea_serial_data_t *)malloc(sizeof(struct nmea_serial_data_t));
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

	data->initialized = 1;
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
	int fd_max;
	fd_set rfds;
	struct message_t msg;
	struct signalfd_siginfo signal_info;

	struct device_t device;
	const struct device_operations_t * ops = NULL;
	struct nmea_serial_data_t * data;

	if (config == NULL)
		return EXIT_FAILURE;

	data = (struct nmea_serial_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	if (!data->initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	ops = &serial_device_operations;
	device_init(&device);

	rc = ops->open(&device, (const struct device_config_t *)&data->serial_config);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to open device '%s'", data->serial_config.name);
		return EXIT_FAILURE;
	}

	while (1) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max)
			fd_max = config->rfd;
		FD_SET(config->signal_fd, &rfds);
		if (config->signal_fd > fd_max)
			fd_max = config->signal_fd;

		rc = select(fd_max + 1, &rfds, NULL, NULL, NULL);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
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
	.help = NULL,
};

