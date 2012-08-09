#include <navcom/destination/logbook.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <sys/select.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <nmea/nmea.h>

static int initialized = 0;

static struct logbook_config_t {
	uint32_t save_timer_id;
	int save_timer_defined;
	char filename[PATH_MAX+1];
	int filename_defined;
} configuration;

static struct information_t {
	struct nmea_angle_t lat;
	char lat_dir;
	struct nmea_angle_t lon;
	char lon_dir;
	struct nmea_time_t time;
	struct nmea_date_t date;
	struct nmea_fix_t heading;
	struct nmea_fix_t speed_over_ground;
} current;

static void set_current_nmea_rmc(const struct nmea_rmc_t * rmc)
{
	switch (rmc->sig_integrity) {
		case NMEA_SIG_INT_AUTONOMOUS:
		case NMEA_SIG_INT_DIFFERENTIAL:
		case NMEA_SIG_INT_ESTIMATED:
		case NMEA_SIG_INT_MANUALINPUT:
			/* those are ok */
			break;
		case NMEA_SIG_INT_SIMULATED:
		case NMEA_SIG_INT_DATANOTVALID:
			/* not accepting these */
			return;
	}

	current.lat = rmc->lat;
	current.lat_dir = rmc->lat_dir;
	current.lon = rmc->lon;
	current.lon_dir = rmc->lon_dir;
	current.time = rmc->time;
	current.date = rmc->date;
	current.heading = rmc->head;
	current.speed_over_ground = rmc->sog;
}

static void process_nmea(const struct nmea_t * nmea)
{
	/* TODO: does the data have to be filtered? sanity check of the current position? */

	switch (nmea->type) {
		case NMEA_RMC:
			set_current_nmea_rmc(&nmea->sentence.rmc);
			break;
		default:
			/* NMEA type not supported by now or interesting at all */
			break;
	}
}

/**
 * Write the log entry to either syslog or log file.
 * All data is separated by semi colons (aka CSV format).
 */
static void write_log(void)
{
	FILE * file = NULL;
	char buf[1024];
	int rc;
	int len;
	int buf_len;
	char * ptr;

	/* TODO: check return of snprintf, right now it is assumed the buffer to be large enough */

	buf_len = (int)sizeof(buf);
	ptr = buf;

	memset(buf, 0, sizeof(buf));

	/* prepare date */
	rc = snprintf(ptr, buf_len, "%04u-%02u-%02u;",
		current.date.y, current.date.m, current.date.d);
	if (rc < 0) {
		syslog(LOG_DEBUG, "error while preparing log entry, rc=%d (at line %d)", rc, __LINE__);
		return;
	} else if (rc < buf_len) {
		ptr += rc;
		buf_len -= rc;
	}

	/* prepare time */
	rc = snprintf(ptr, buf_len, "%02u-%02u-%02u;",
		current.time.h, current.time.m, current.date.d);
	if (rc < 0) {
		syslog(LOG_DEBUG, "error while preparing log entry, rc=%d (at line %d)", rc, __LINE__);
		return;
	} else if (rc < buf_len) {
		ptr += rc;
		buf_len -= rc;
	}

	/* prepare latitude */
	rc = snprintf(ptr, buf_len, "%02u-%02u,%1u%c;",
		current.lat.d, current.lat.m, (current.lat.s.i * 100) / 60, current.lat_dir);
	if (rc < 0) {
		syslog(LOG_DEBUG, "error while preparing log entry, rc=%d (at line %d)", rc, __LINE__);
		return;
	} else if (rc < buf_len) {
		ptr += rc;
		buf_len -= rc;
	}

	/* prepare longitude */
	rc = snprintf(ptr, buf_len, "%03u-%02u,%1u%c;",
		current.lon.d, current.lon.m, (current.lon.s.i * 100) / 60, current.lon_dir);
	if (rc < 0) {
		syslog(LOG_DEBUG, "error while preparing log entry, rc=%d (at line %d)", rc, __LINE__);
		return;
	} else if (rc < buf_len) {
		ptr += rc;
		buf_len -= rc;
	}

	/* TODO: prepare heading */
	/* TODO: prepare speed over ground */
	/* TODO: prepare wind speed (average, min/max in last period) */
	/* TODO: prepare wind direction */

	len = (int)strlen(buf);

	/* write entry to syslog or file */
	if (configuration.filename_defined) {
		file = fopen(configuration.filename, "wt+");
		if (!file) {
			syslog(LOG_ERR, "cannot open logbook file '%s', error: %s", configuration.filename, strerror(errno));
			return;
		}
		rc = fprintf(file, "%s\n", buf);
		fclose(file);
		if (rc != len + 1) {
			syslog(LOG_DEBUG, "error while writing logbook entry: %d bytes written, should have %u", rc, len + 1);
		}
	} else {
		syslog(LOG_INFO, "logbook: %s", buf);
	}
}

static void process_timer(uint32_t id)
{
	if (!configuration.save_timer_defined) return;
	if (id != configuration.save_timer_id) return;
	write_log();
}

static int read_save_timer(const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	char * endptr;

	prop = proplist_find(properties, "save_timer_id");
	if (prop) {
		configuration.save_timer_id = strtoul(prop->value, &endptr, 0);
		if (*endptr != '0') {
			syslog(LOG_ERR, "invalid save_timer_id: '%s'", prop->key);
			return EXIT_FAILURE;
		}
		configuration.save_timer_defined = 1;
	} else {
		configuration.save_timer_defined = 0;
	}
	return EXIT_SUCCESS;
}

static int read_filename(const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	size_t len;

	prop = proplist_find(properties, "filename");
	if (!prop) return EXIT_FAILURE;

	len = strlen(prop->value);
	if (len > sizeof(configuration.filename) - 1) {
		return EXIT_FAILURE;
	}

	if (len > 0) {
		strncpy(configuration.filename, prop->value, sizeof(configuration.filename) - 1);
		configuration.filename_defined = 1;
	} else {
		configuration.filename_defined = 0;
	}

	return EXIT_SUCCESS;
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	memset(&configuration, 0, sizeof(configuration));

	if (read_save_timer(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_filename(properties) != EXIT_SUCCESS) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static int proc(const struct proc_config_t * config)
{
	int rc;
	fd_set rfds;
	struct message_t msg;

	if (!initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				syslog(LOG_ERR, "unable to read from pipe: %s", strerror(errno));
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
					process_nmea(&msg.data.nmea);
					break;

				case MSG_TIMER:
					process_timer(msg.data.timer_id);
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

const struct proc_desc_t logbook = {
	.name = "logbook",
	.configure = configure,
	.func = proc
};

