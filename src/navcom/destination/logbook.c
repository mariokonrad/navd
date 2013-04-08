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
#include <math.h>
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

	/* minimal distance change to write log entry in meters */
	long min_meter_position_change;
} configuration;

static struct information_t {
	/* bookkeeping information */
	struct timeval last_update;

	/* navigation information */
	struct nmea_angle_t lat;
	char lat_dir;
	struct nmea_angle_t lon;
	char lon_dir;
	struct nmea_time_t time;
	struct nmea_date_t date;
	struct nmea_fix_t course_over_ground;
	struct nmea_fix_t speed_over_ground;
	struct nmea_fix_t course_magnetic;
	struct nmea_fix_t speed_through_water;
	struct nmea_fix_t wind_speed;
	struct nmea_fix_t wind_direction;
	uint32_t pressure; /* units of 0.1 mbar */
	int32_t air_temperature; /* units of degree celcius */
} current, last_written_data;

/**
 * Returns true if the specified time is not zero.
 */
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

static double sqr(double x)
{
	return x * x;
}

/**
 * Returns the number of meters the positon has changed since the last
 * write update.
 *
 * @retval    -1 Unable to calculate distance
 * @retval other Distance in meters
 */
static long diff_position(void)
{
	static const double EARTH_RADIUS = 6378000.0; /* [m] mean radius */

	double lat_0 = 0.0;
	double lat_1 = 0.0;
	double lon_0 = 0.0;
	double lon_1 = 0.0;

	nmea_angle_to_double(&lat_0, &last_written_data.lat);
	nmea_angle_to_double(&lon_0, &last_written_data.lon);
	nmea_angle_to_double(&lat_1, &current.lat);
	nmea_angle_to_double(&lon_1, &current.lon);

	if (last_written_data.lat_dir != 'N') lat_0 = -lat_0;
	if (last_written_data.lon_dir != 'W') lon_0 = -lon_0;
	if (current.lat_dir != 'N') lat_1 = -lat_1;
	if (current.lon_dir != 'W') lon_1 = -lon_1;

	/* calculate distance in meters on sphere, precise enough approximation */

	return (long)round(EARTH_RADIUS * atan(sqrt(
		sqr(cos(lat_1) * sin(lon_1 - lon_0))
			+ sqr(cos(lat_0) * sin(lat_1) - sin(lat_1) * cos(lat_1) * cos(lon_1 - lon_0)))
		/ (sin(lat_0) * sin(lat_1) + cos(lat_0) * cos(lat_1) * cos(lon_1 - lon_0))));
}

/**
 * Determines whether to accept or not the signal integrity. Currently
 * only the following are accepted:
 * - autonomous
 * - differential
 * - estimated
 * - manual input
 */
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

/**
 * @todo Add more NMEA sentences (wind, sounder, etc.)
 */
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

static int prepare_date(char * ptr, int len)
{
	return snprintf(ptr, len, "%04u-%02u-%02u;",
		current.date.y, current.date.m, current.date.d);
}

static int prepare_time(char * ptr, int len)
{
	return snprintf(ptr, len, "%02u-%02u-%02u;",
		current.time.h, current.time.m, current.time.s);
}

static int prepare_latitude(char * ptr, int len)
{
	return snprintf(ptr, len, "%02u-%02u,%1u%c;",
		current.lat.d, current.lat.m, (current.lat.s.i * 100) / 60, current.lat_dir);
}

static int prepare_longitude(char * ptr, int len)
{
	return snprintf(ptr, len, "%03u-%02u,%1u%c;",
		current.lon.d, current.lon.m, (current.lon.s.i * 100) / 60, current.lon_dir);
}

static int prepare_course_over_ground(char * ptr, int len)
{
	return snprintf(ptr, len, "%u,%02u;",
		current.course_over_ground.i, current.course_over_ground.d);
}

static int prepare_speed_over_ground(char * ptr, int len)
{
	return snprintf(ptr, len, "%u,%02u;",
		current.speed_over_ground.i, current.speed_over_ground.d);
}

static int prepare_course_magnetic(char * ptr, int len)
{
	return snprintf(ptr, len, "%u,%02u;",
		current.course_magnetic.i, current.course_magnetic.d);
}

static int prepare_speed_through_water(char * ptr, int len)
{
	return snprintf(ptr, len, "%u,%02u;",
		current.speed_through_water.i, current.speed_through_water.d);
}

static int prepare_wind_speed(char * ptr, int len)
{
	return snprintf(ptr, len, "%u,%02u;",
		current.wind_speed.i, current.wind_speed.d);
}

static int prepare_wind_direction(char * ptr, int len)
{
	return snprintf(ptr, len, "%u;",
		current.wind_direction.i);
}

static int prepare_pressure(char * ptr, int len)
{
	return snprintf(ptr, len, "%u;",
		current.pressure);
}

static int prepare_air_temperature(char * ptr, int len)
{
	return snprintf(ptr, len, "%d;",
		current.air_temperature);
}

/**
 * @retval  0 Success, do write update
 * @retval -1 Unable to calculate update time, do not write
 * @retval -2 Last position update too long ago, do not write
 */
static int check_age_last_update(void)
{
	long dt;

	dt = elapsed_ms_time(&current.last_update);
	if (dt < 0) {
		syslog(LOG_WARNING, "not writing log, cannot calculate update time");
		return -1;
	}
	if (dt > configuration.timeout_for_writing) {
		syslog(LOG_WARNING, "not writing log, last position update %ld msec ago", dt);
		return -2;
	}
	return 0;
}

/**
 * @retval  0 Position changed considerably, do write log
 * @retval -1 Unable to calculate position change, do not write log entry
 * @retval -2 No significant position change, log entry not worth it
 */
static int check_position_change(void)
{
	long ds = 0;

	if (configuration.min_meter_position_change <= 0)
		return 0; /* check disabled, always write log */

	ds = diff_position();

	if (ds < 0) {
		syslog(LOG_WARNING, "not writing log, cannot calculate position change");
		return -1;
	}
	if (ds < configuration.min_meter_position_change) {
		syslog(LOG_INFO, "not writing log, position has not changed significantly");
		return -2;
	}
	return 0;
}

/**
 * Write the log entry to either syslog or log file.
 * All data is separated by semi colons (aka CSV format).
 */
static void write_log(void)
{
	typedef int (*prepare_func_t)(char *, int);
	typedef int (*check_func_t)(void);

	static const check_func_t CHECK[] =
	{
		check_age_last_update,
		check_position_change,
	};

	static const prepare_func_t PREPARE[] =
	{
		prepare_date,
		prepare_time,
		prepare_latitude,
		prepare_longitude,
		prepare_course_over_ground,
		prepare_speed_over_ground,
		prepare_course_magnetic,
		prepare_speed_through_water,
		prepare_wind_speed,
		prepare_wind_direction,
		prepare_pressure,
		prepare_air_temperature,
	};

	FILE * file = NULL;
	char buf[1024];
	int rc;
	int len;
	int buf_len;
	char * ptr;
	size_t i;

	/* perform checks */

	for (i = 0; i < sizeof(CHECK) / sizeof(CHECK[0]); ++i) {
		if (CHECK[i]())
			return;
	}

	/* prepare log entry */

	buf_len = (int)sizeof(buf);
	ptr = buf;
	memset(buf, 0, sizeof(buf));

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

	last_written_data = current;
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

static int read_min_position_change(const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "min_position_change");
	configuration.min_meter_position_change = 0; /* default value */
	if (prop != NULL) {
		long tmp = strtol(prop->value, &endptr, 0);
		if (*endptr != '\0') {
			syslog(LOG_ERR, "invalid value in timeout: '%s'", prop->value);
			return EXIT_FAILURE;
		}
		if (tmp < 0) {
			syslog(LOG_ERR, "invalid value for timeout: %ld, must be greater than zero", tmp);
			return EXIT_FAILURE;
		}
		configuration.min_meter_position_change = tmp;
	}
	return EXIT_SUCCESS;
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);

	memset(&configuration, 0, sizeof(configuration));
	memset(&current, 0, sizeof(current));
	memset(&last_written_data, 0, sizeof(last_written_data));

	if (read_save_timer(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_filename(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_timeout(properties) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (read_min_position_change(properties) != EXIT_SUCCESS) return EXIT_FAILURE;

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

