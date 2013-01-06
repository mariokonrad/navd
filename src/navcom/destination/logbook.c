#include <navcom/destination/logbook.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <sys/select.h>
#include <sys/time.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>

#include <nmea/nmea.h>

static bool initialized = false;

static struct logbook_config_t {
	uint32_t save_timer_id;
	int save_timer_defined;
	char filename[PATH_MAX+1];
	int filename_defined;

	/* timeout for the last reported position, if the position is older, no log will be written */
	long timeout_for_writing;
} configuration;

static struct information_t {
	/* bookkeeping information */
	struct timeval last_update;

	/* NMEA information */
	struct nmea_angle_t lat;
	char lat_dir;
	struct nmea_angle_t lon;
	char lon_dir;
	struct nmea_time_t time;
	struct nmea_date_t date;
	struct nmea_fix_t course_over_ground;
	struct nmea_fix_t speed_over_ground;
} current;

static bool time_valid(const struct timeval * t)
{
	return true
		&& (t != NULL)
		&& (t->tv_sec > 0)
		&& (t->tv_usec > 0)
		;
}

static long diff_msec(const struct timeval * ts, const struct timeval * te)
{
	long dt = 0;

	if (ts == NULL) return -1;
	if (te == NULL) return -1;

	dt = (te->tv_sec - ts->tv_sec) * 1000;
	dt += (te->tv_usec - ts->tv_usec) / 1000;

	return (dt >= 0) ? dt : -2;
}

/**
 * Returns the elapsed milliseconds since the last update.
 *
 * @param[in] tm Time of the last update.
 * @retval    -1 Parameter failure.
 * @retval    -2 Provided timestamps not valid.
 * @retval other Elapsed time in milliseconds since specified timestamp.
 */
static long elapsed_ms_time(const struct timeval * tm)
{
	long dt = 0;
	int rc;
	struct timeval t;

	if (tm == NULL) return -1;
	if (!time_valid(tm)) return -1;

	rc = gettimeofday(&t, NULL);
	if (rc < 0) {
		syslog(LOG_ERR, "error while reading timestamp, errno=%d", errno);
		return -1;
	}

	dt = diff_msec(tm, &t);
	if (dt < 0) return -2;

	return dt;
}

static bool accept_signal_integrity(char sig_integrity)
{
	switch (sig_integrity) {
		case NMEA_SIG_INT_AUTONOMOUS:
		case NMEA_SIG_INT_DIFFERENTIAL:
		case NMEA_SIG_INT_ESTIMATED:
		case NMEA_SIG_INT_MANUALINPUT:
			/* those are ok */
			return true;
		case NMEA_SIG_INT_SIMULATED:
		case NMEA_SIG_INT_DATANOTVALID:
			/* not accepting these */
			break;
	}
	return false;
}

/**
 * Sets the current position (RMC sentence) if the signal integrity is
 * accepted and the last update is not too long since.
 */
static void set_current_nmea_rmc(const struct nmea_rmc_t * rmc)
{
	int rc;
	struct timeval last_update;

	if (!accept_signal_integrity(rmc->sig_integrity)) return;

	/* timestamp of last update */
	rc = gettimeofday(&last_update, NULL);
	if (rc < 0) {
		syslog(LOG_ERR, "error while reading timestamp, errno=%d", errno);
		return;
	}
	current.last_update = last_update;

	/* position information */
	current.lat = rmc->lat;
	current.lat_dir = rmc->lat_dir;
	current.lon = rmc->lon;
	current.lon_dir = rmc->lon_dir;
	current.time = rmc->time;
	current.date = rmc->date;
	current.course_over_ground = rmc->head;
	current.speed_over_ground = rmc->sog;
}

static void process_nmea(const struct nmea_t * nmea)
{
	switch (nmea->type) {
		case NMEA_RMC:
			set_current_nmea_rmc(&nmea->sentence.rmc);
			break;
		default:
			/* NMEA type not supported by now or not interesting at all */
			break;
	}
}

static int prepare_date(char * ptr, int buf_len)
{
	return snprintf(ptr, buf_len, "%04u-%02u-%02u;",
		current.date.y, current.date.m, current.date.d);
}

static int prepare_time(char * ptr, int buf_len)
{
	return snprintf(ptr, buf_len, "%02u-%02u-%02u;",
		current.time.h, current.time.m, current.time.s);
}

static int prepare_latitude(char * ptr, int buf_len)
{
	return snprintf(ptr, buf_len, "%02u-%02u,%1u%c;",
		current.lat.d, current.lat.m, (current.lat.s.i * 100) / 60, current.lat_dir);
}

static int prepare_longitude(char * ptr, int buf_len)
{
	return snprintf(ptr, buf_len, "%03u-%02u,%1u%c;",
		current.lon.d, current.lon.m, (current.lon.s.i * 100) / 60, current.lon_dir);
}

/**
 * Write the log entry to either syslog or log file.
 * All data is separated by semi colons (aka CSV format).
 */
static void write_log(void)
{
	typedef int (*prepare_func_t)(char *, int);

	static const prepare_func_t PREPARE[] =
	{
		prepare_date,
		prepare_time,
		prepare_latitude,
		prepare_longitude,
		/* TODO: prepare course over ground */
		/* TODO: prepare speed over ground */
		/* TODO: prepare course through water */
		/* TODO: prepare speed through water */
		/* TODO: prepare wind speed (average, min/max in last period) */
		/* TODO: prepare wind direction */
	};

	FILE * file = NULL;
	char buf[1024];
	int rc;
	int len;
	int buf_len;
	char * ptr;
	long dt;
	size_t i;

	/* TODO: honor minimum time of logging period of n minutes, no matter how the timer is configured, this timeout should be configurable */

	/* check for age of last update, if to old, writing log makes no sense */
	dt = elapsed_ms_time(&current.last_update);
	if (dt < 0) {
		syslog(LOG_WARNING, "not writing log, cannot calculate update time");
		return;
	} else if (dt > configuration.timeout_for_writing) {
		syslog(LOG_WARNING, "not writing log, last position update %ld msec ago", dt);
		return;
	}

	/* TODO: check for changes in position, if none, there is no need to write a log */

	buf_len = (int)sizeof(buf);
	ptr = buf;

	memset(buf, 0, sizeof(buf));

	/* prepare log entry */

	for (i = 0; i < sizeof(PREPARE) / sizeof(PREPARE[0]); ++i) {
		rc = PREPARE[i](ptr, buf_len);
		if (rc < 0) {
			syslog(LOG_DEBUG, "error while preparing log entry, rc=%d (at line %d)", rc, __LINE__);
			return;
		} else if (rc < buf_len) {
			ptr += rc;
			buf_len -= rc;
		} else {
			syslog(LOG_ERR, "error while evaluating date format string, buffer not large enough: %lu at line %d", sizeof(buf), __LINE__);
			return;
		}
	}

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
		if (*endptr != '\0') {
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

static int read_timeout(const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "write_timeout");
	configuration.timeout_for_writing = 5; /* default value */
	if (prop != NULL) {
		long tmp = strtol(prop->value, &endptr, 0);
		if (*endptr != '\0') {
			syslog(LOG_ERR, "invalid value in timeout: '%s'", prop->value);
			return EXIT_FAILURE;
		}
		if (tmp <= 0) {
			syslog(LOG_ERR, "invalid value for timeout: %ld, must be greater than zero", tmp);
			return EXIT_FAILURE;
		}
		configuration.timeout_for_writing = tmp;
	}
	configuration.timeout_for_writing *= 1000; /* unit: seconds */
	return EXIT_SUCCESS;
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	memset(&configuration, 0, sizeof(configuration));
	memset(&current, 0, sizeof(current));

	if (read_save_timer(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_filename(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_timeout(properties) != EXIT_SUCCESS) return EXIT_FAILURE;

	initialized = true;
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

