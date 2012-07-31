#include <navcom/gps_simulator.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <errno.h>
#include <sys/select.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>

static struct message_t sim_message;

static struct {
	uint32_t period; /* seconds */
	uint32_t sog; /* tenth of degrees */
	uint32_t heading; /* tenth of degrees */
	uint32_t mag; /* tenth of degrees */
} option = {
	.period = 1,
	.sog = 0,
	.heading = 0,
	.mag = 0,
};

static void init_message(void)
{
	struct nmea_rmc_t * rmc;

	sim_message.type = MSG_NMEA;
	sim_message.data.nmea.type = NMEA_RMC;
	rmc = &sim_message.data.nmea.sentence.rmc;
	rmc->time.h = 20;
	rmc->time.m = 15;
	rmc->time.s = 30;
	rmc->time.ms = 0;
	rmc->status = NMEA_STATUS_OK;
	rmc->lat.d = 47;
	rmc->lat.m = 8;
	rmc->lat.s.i = 0;
	rmc->lat.s.d = 0;
	rmc->lat_dir = NMEA_NORTH;
	rmc->lon.d = 3;
	rmc->lon.m = 20;
	rmc->lon.s.i = 0;
	rmc->lon.s.d = 0;
	rmc->lon_dir = NMEA_EAST;
	rmc->sog.i = option.sog / 10;
	rmc->sog.d = option.sog % 10;
	rmc->head.i = option.heading / 10;
	rmc->head.d = option.heading % 10;
	rmc->date.y = 0;
	rmc->date.m = 1;
	rmc->date.d = 1;
	rmc->m.i = option.mag / 10;
	rmc->m.d = option.mag % 10;
	rmc->m_dir = NMEA_WEST;
	rmc->sig_integrity = NMEA_SIG_INT_SIMULATED;
}

static int read_prop_uint32(const struct property_list_t * properties, const char * key, uint32_t * value)
{
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, key);
	if (prop) {
		*value = strtoul(prop->value, &endptr, 0);
		if (*endptr != '\0') {
			syslog(LOG_ERR, "invalid value in '%s': '%s'", prop->key, prop->value);
			return EXIT_FAILURE;
		}
	} else {
		syslog(LOG_DEBUG, "property '%s' not defined, using default of %u s", key, *value);
	}
	return EXIT_SUCCESS;
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	if (read_prop_uint32(properties, "period",  &option.period)  != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_prop_uint32(properties, "sog",     &option.sog)     != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_prop_uint32(properties, "heading", &option.heading) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_prop_uint32(properties, "mag",     &option.mag)     != EXIT_SUCCESS) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int proc(const struct proc_config_t * config)
{
	fd_set rfds;
	struct message_t msg;
	int rc;
	struct timespec tm;

	char buf[NMEA_MAX_SENTENCE];

	/* TODO: properties: position latitude */
	/* TODO: properties: position longitude */
	/* TODO: properties: time */
	/* TODO: properties: date */

	init_message();

	/* prepare send buffer */
	rc = nmea_write(buf, sizeof(buf), &sim_message.data.nmea);
	if (rc < 0) {
		syslog(LOG_WARNING, "invalid RMC data, rc=%d", rc);
		return EXIT_FAILURE;
	}

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		tm.tv_sec = option.period;
		tm.tv_nsec = 0;

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timeout */
			rc = write(config->wfd, &sim_message, sizeof(sim_message));
			if (rc < 0) {
				perror("write");
				syslog(LOG_DEBUG, "wfd=%d", config->wfd);
			}
			continue;
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
	.configure = configure,
	.func = proc
};

