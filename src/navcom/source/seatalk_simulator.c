#include <navcom/source/seatalk_simulator.h>
#include <navcom/property_read.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <errno.h>
#include <sys/select.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct seatalk_simulator_data_t {
	uint32_t depth; /* feet */
	uint32_t period; /* milliseconds */
};

static void init_data(struct seatalk_simulator_data_t * data)
{
	memset(data, 0, sizeof(struct seatalk_simulator_data_t));
	data->depth = 0;
}

static void init_message(
		struct message_t * msg,
		const struct seatalk_simulator_data_t * data)
{
	UNUSED_ARG(data);

	msg->type = MSG_SEATALK;

	/* TODO: fill seatalk part of message */
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	struct seatalk_simulator_data_t * data;

	if (config == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct seatalk_simulator_data_t *)malloc(sizeof(struct seatalk_simulator_data_t));
	config->data = data;
	init_data(data);

	if (property_read_uint32(properties, "depth", &data->depth)  != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "period", &data->period)  != EXIT_SUCCESS)
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
	fd_set rfds;
	struct message_t msg;
	int rc;
	struct timespec tm;
	struct message_t sim_message;
	struct seatalk_simulator_data_t * data;

/*
	char buf[NMEA_MAX_SENTENCE];
*/

	if (!config)
		return EXIT_FAILURE;

	data = (struct seatalk_simulator_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	init_message(&sim_message, data);

	/* TODO: prepare send buffer */
/*
	rc = nmea_write(buf, sizeof(buf), &sim_message.data.nmea);
	if (rc < 0) {
		syslog(LOG_WARNING, "invalid RMC data, rc=%d", rc);
		return EXIT_FAILURE;
	}
*/

	while (!proc_request_terminate()) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		tm.tv_sec = data->period;
		tm.tv_nsec = 0;

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, &tm, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) /* timeout */
			if (message_write(config->wfd, &sim_message) != EXIT_SUCCESS)
				return EXIT_FAILURE;

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
				default:
					break;
			}
			continue;
		}
	}
	return EXIT_SUCCESS;
}

const struct proc_desc_t seatalk_simulator = {
	.name = "seatalk_sim",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = NULL,
};

