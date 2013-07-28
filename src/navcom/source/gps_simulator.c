#include <navcom/source/gps_simulator.h>
#include <navcom/property_read.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct gps_simulator_data_t {
	uint32_t period; /* seconds */
	uint32_t sog; /* tenth of knots */
	uint32_t heading; /* tenth of degrees */
	uint32_t mag; /* tenth of degrees */
	struct nmea_date_t date;
	struct nmea_time_t time;
	struct nmea_angle_t lat;
	struct nmea_angle_t lon;
	uint32_t simulated; /* simulation or not */
};

static void init_data(struct gps_simulator_data_t * data)
{
	memset(data, 0, sizeof(struct gps_simulator_data_t));
	data->period = 1;
	data->date.y = 2000;
	data->date.m = 1;
	data->date.d = 1;
	data->simulated = 1;
}

static void init_message(
		struct message_t * msg,
		const struct gps_simulator_data_t * data)
{
	struct nmea_rmc_t * rmc;

	msg->type = MSG_NMEA;
	msg->data.nmea.type = NMEA_RMC;
	rmc = &msg->data.nmea.sentence.rmc;
	rmc->time = data->time;
	rmc->status = NMEA_STATUS_OK;
	rmc->lat = data->lat;
	rmc->lat_dir = NMEA_NORTH;
	rmc->lon = data->lon;
	rmc->lon_dir = NMEA_EAST;
	rmc->sog.i = data->sog / 10;
	rmc->sog.d = (data->sog % 10) * NMEA_FIX_DECIMALS;
	rmc->head.i = data->heading / 10;
	rmc->head.d = (data->heading % 10) * NMEA_FIX_DECIMALS;
	rmc->date = data->date;
	rmc->m.i = data->mag / 10;
	rmc->m.d = (data->mag % 10) * NMEA_FIX_DECIMALS;
	rmc->m_dir = NMEA_WEST;
	rmc->sig_integrity = data->simulated ? NMEA_SIG_INT_SIMULATED : NMEA_SIG_INT_AUTONOMOUS;
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	struct gps_simulator_data_t * data;
	const struct property_t * prop = NULL;
	uint32_t min_dec;
	int rc;

	if (config == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct gps_simulator_data_t *)malloc(sizeof(struct gps_simulator_data_t));
	config->data = data;
	init_data(data);

	if (property_read_uint32(properties, "period",  &data->period)  != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "sog",     &data->sog)     != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "heading", &data->heading) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "mag",     &data->mag)     != EXIT_SUCCESS)
		return EXIT_FAILURE;

	prop = proplist_find(properties, "date");
	if (prop) {
		rc = sscanf(prop->value, "%04u-%02u-%02u", &data->date.y, &data->date.m, &data->date.d);
		if (rc != 3) {
			syslog(LOG_WARNING, "invalid date '%s', using default", prop->value);
			data->date.y = 2000;
			data->date.m = 1;
			data->date.d = 1;
		}
	}

	prop = proplist_find(properties, "time");
	if (prop) {
		rc = sscanf(prop->value, "%02u-%02u-%02u", &data->time.h, &data->time.m, &data->time.s);
		if (rc != 3) {
			syslog(LOG_WARNING, "invalid time '%s', using default", prop->value);
			data->time.h = 0;
			data->time.m = 0;
			data->time.s = 0;
		}
		data->time.ms = 0;
	}

	prop = proplist_find(properties, "lat");
	if (prop) {
		rc = sscanf(prop->value, "%02u-%02u,%3u", &data->lat.d, &data->lat.m, &min_dec);
		if (rc != 3) {
			syslog(LOG_INFO, "invalid latitude: '%s', using default", prop->value);
			data->lat.d = 0;
			data->lat.m = 0;
			data->lat.s.i = 0;
			data->lat.s.d = 0;
		} else {
			data->lat.s.i = (min_dec * 60) / 100;
			data->lat.s.d = 0;
		}
	}

	prop = proplist_find(properties, "lon");
	if (prop) {
		rc = sscanf(prop->value, "%03u-%02u,%3u", &data->lon.d, &data->lon.m, &min_dec);
		if (rc != 3) {
			syslog(LOG_INFO, "invalid latitude: '%s', using default", prop->value);
			data->lon.d = 0;
			data->lon.m = 0;
			data->lon.s.i = 0;
			data->lon.s.d = 0;
		} else {
			data->lon.s.i = (min_dec * 60) / 100;
			data->lon.s.d = 0;
		}
	}

	/* turn off simulation, ATTENTION: this is for experts use only */
	prop = proplist_find(properties, "__not_simulated__");
	if (prop) {
		data->simulated = 0;
	}

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
	int fd_max;
	struct message_t msg;
	int rc;
	struct timeval tm;
	struct message_t sim_message;
	struct gps_simulator_data_t * data;
	struct signalfd_siginfo signal_info;

	char buf[NMEA_MAX_SENTENCE];

	if (!config)
		return EXIT_FAILURE;

	data = (struct gps_simulator_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	init_message(&sim_message, data);

	/* prepare send buffer */
	rc = nmea_write(buf, sizeof(buf), &sim_message.data.nmea);
	if (rc < 0) {
		syslog(LOG_WARNING, "invalid RMC data, rc=%d", rc);
		return EXIT_FAILURE;
	}

	while (1) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max)
			fd_max = -1;
		FD_SET(config->signal_fd, &rfds);
		if (config->signal_fd > fd_max)
			fd_max = config->signal_fd;

		tm.tv_sec = data->period;
		tm.tv_usec = 0;

		rc = select(fd_max, &rfds, NULL, NULL, &tm);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) /* timeout */
			if (message_write(config->wfd, &sim_message) != EXIT_SUCCESS)
				return EXIT_FAILURE;

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
				default:
					break;
			}
			continue;
		}
	}
	return EXIT_SUCCESS;
}

const struct proc_desc_t gps_simulator = {
	.name = "gps_sim",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = NULL,
};

