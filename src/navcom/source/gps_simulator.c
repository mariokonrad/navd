#include <navcom/source/gps_simulator.h>
#include <navcom/property_read.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <errno.h>
#include <sys/select.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct option_t {
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

/* TODO: move static data into config->data */
static struct option_t option = {
	.period = 1,
	.sog = 0,
	.heading = 0,
	.mag = 0,
	.date = { 2000, 1, 1 },
	.time = { 0, 0, 0, 0 },
	.lat = { 0, 0, { 0, 0 } },
	.lon = { 0, 0, { 0, 0 } },
	.simulated = 1,
};

static void init_message(struct message_t * msg)
{
	struct nmea_rmc_t * rmc;

	msg->type = MSG_NMEA;
	msg->data.nmea.type = NMEA_RMC;
	rmc = &msg->data.nmea.sentence.rmc;
	rmc->time = option.time;
	rmc->status = NMEA_STATUS_OK;
	rmc->lat = option.lat;
	rmc->lat_dir = NMEA_NORTH;
	rmc->lon = option.lon;
	rmc->lon_dir = NMEA_EAST;
	rmc->sog.i = option.sog / 10;
	rmc->sog.d = (option.sog % 10) * NMEA_FIX_DECIMALS;
	rmc->head.i = option.heading / 10;
	rmc->head.d = (option.heading % 10) * NMEA_FIX_DECIMALS;
	rmc->date = option.date;
	rmc->m.i = option.mag / 10;
	rmc->m.d = (option.mag % 10) * NMEA_FIX_DECIMALS;
	rmc->m_dir = NMEA_WEST;
	rmc->sig_integrity = option.simulated ? NMEA_SIG_INT_SIMULATED : NMEA_SIG_INT_AUTONOMOUS;
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	uint32_t min_dec;
	int rc;

	UNUSED_ARG(config);

	if (property_read_uint32(properties, "period",  &option.period)  != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "sog",     &option.sog)     != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "heading", &option.heading) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (property_read_uint32(properties, "mag",     &option.mag)     != EXIT_SUCCESS)
		return EXIT_FAILURE;

	prop = proplist_find(properties, "date");
	if (prop) {
		rc = sscanf(prop->value, "%04u-%02u-%02u", &option.date.y, &option.date.m, &option.date.d);
		if (rc != 3) {
			syslog(LOG_WARNING, "invalid date '%s', using default", prop->value);
			option.date.y = 2000;
			option.date.m = 1;
			option.date.d = 1;
		}
	}

	prop = proplist_find(properties, "time");
	if (prop) {
		rc = sscanf(prop->value, "%02u-%02u-%02u", &option.time.h, &option.time.m, &option.time.s);
		if (rc != 3) {
			syslog(LOG_WARNING, "invalid time '%s', using default", prop->value);
			option.time.h = 0;
			option.time.m = 0;
			option.time.s = 0;
		}
		option.time.ms = 0;
	}

	prop = proplist_find(properties, "lat");
	if (prop) {
		rc = sscanf(prop->value, "%02u-%02u,%3u", &option.lat.d, &option.lat.m, &min_dec);
		if (rc != 3) {
			syslog(LOG_INFO, "invalid latitude: '%s', using default", prop->value);
			option.lat.d = 0;
			option.lat.m = 0;
			option.lat.s.i = 0;
			option.lat.s.d = 0;
		} else {
			option.lat.s.i = (min_dec * 60) / 100;
			option.lat.s.d = 0;
		}
	}

	prop = proplist_find(properties, "lon");
	if (prop) {
		rc = sscanf(prop->value, "%03u-%02u,%3u", &option.lon.d, &option.lon.m, &min_dec);
		if (rc != 3) {
			syslog(LOG_INFO, "invalid latitude: '%s', using default", prop->value);
			option.lon.d = 0;
			option.lon.m = 0;
			option.lon.s.i = 0;
			option.lon.s.d = 0;
		} else {
			option.lon.s.i = (min_dec * 60) / 100;
			option.lon.s.d = 0;
		}
	}

	/* turn off simulation, ATTENTION: this is for experts use only */
	prop = proplist_find(properties, "__not_simulated__");
	if (prop) {
		option.simulated = 0;
	}

	return EXIT_SUCCESS;
}

static int exit_proc(struct proc_config_t * config)
{
	UNUSED_ARG(config);
	return EXIT_SUCCESS;
}

static int proc(struct proc_config_t * config)
{
	fd_set rfds;
	struct message_t msg;
	int rc;
	struct timespec tm;
	struct message_t sim_message;

	char buf[NMEA_MAX_SENTENCE];

	init_message(&sim_message);

	/* prepare send buffer */
	rc = nmea_write(buf, sizeof(buf), &sim_message.data.nmea);
	if (rc < 0) {
		syslog(LOG_WARNING, "invalid RMC data, rc=%d", rc);
		return EXIT_FAILURE;
	}

	while (!proc_request_terminate()) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		tm.tv_sec = option.period;
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

const struct proc_desc_t gps_simulator = {
	.name = "gps_sim",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
};

