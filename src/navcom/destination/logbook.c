#include <navcom/destination/logbook.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <sys/select.h>
#include <sys/signalfd.h>
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

/* TODO: configuration of columns in config file, providing a default structure */

struct logbook_config_t {
	uint32_t save_timer_id;
	int save_timer_defined;
	char filename[PATH_MAX+1];
	int filename_defined;

	/* timeout for the last reported position, if the position is older, no log will be written */
	long timeout_for_writing;

	/* minimal distance change to write log entry in meters */
	long min_meter_position_change;
};

struct information_t {
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
};

struct logbook_data_t
{
	bool initialized;
	struct logbook_config_t configuration;
	struct information_t current;
	struct information_t last_written_data;
};

static void init_data(struct logbook_data_t * data)
{
	memset(data, 0, sizeof(struct logbook_data_t));
}

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

static long diff_msec(
		const struct timeval * ts,
		const struct timeval * te)
{
	long dt = 0;

	if (ts == NULL)
		return -1;
	if (te == NULL)
		return -1;

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

	if (tm == NULL)
		return -1;
	if (!time_valid(tm))
		return -1;

	rc = gettimeofday(&t, NULL);
	if (rc < 0) {
		syslog(LOG_ERR, "error while reading timestamp, errno=%d", errno);
		return -1;
	}

	dt = diff_msec(tm, &t);
	if (dt < 0)
		return -2;

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
 * @param[in] last Last written information.
 * @param[in] curr Current information to compare.
 * @retval    -1 Unable to calculate distance
 * @retval other Distance in meters
 */
static long diff_position(
		const struct information_t * last,
		const struct information_t * curr)
{
	static const double EARTH_RADIUS = 6378000.0; /* [m] mean radius */

	double lat_0 = 0.0;
	double lat_1 = 0.0;
	double lon_0 = 0.0;
	double lon_1 = 0.0;

	nmea_angle_to_double(&lat_0, &last->lat);
	nmea_angle_to_double(&lon_0, &last->lon);
	nmea_angle_to_double(&lat_1, &curr->lat);
	nmea_angle_to_double(&lon_1, &curr->lon);

	if (last->lat_dir != 'N')
		lat_0 = -lat_0;
	if (last->lon_dir != 'W')
		lon_0 = -lon_0;
	if (curr->lat_dir != 'N')
		lat_1 = -lat_1;
	if (curr->lon_dir != 'W')
		lon_1 = -lon_1;

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
 *
 * @param[out] current Structure to contain current information.
 * @param[in] rmc Data to remember within the current information.
 */
static void set_current_nmea_rmc(
		struct information_t * current,
		const struct nmea_rmc_t * rmc)
{
	int rc;
	struct timeval last_update;

	if (!accept_signal_integrity(rmc->sig_integrity))
		return;

	/* timestamp of last update */
	rc = gettimeofday(&last_update, NULL);
	if (rc < 0) {
		syslog(LOG_ERR, "error while reading timestamp, errno=%d", errno);
		return;
	}
	current->last_update = last_update;

	/* position information */
	current->lat = rmc->lat;
	current->lat_dir = rmc->lat_dir;
	current->lon = rmc->lon;
	current->lon_dir = rmc->lon_dir;
	current->time = rmc->time;
	current->date = rmc->date;
	current->course_over_ground = rmc->head;
	current->speed_over_ground = rmc->sog;
}

/**
 * @todo Add NMEA sentence: wind [$IIMWV]
 * @todo Add NMEA sentence: depth sounder [$IIDBT, $IIDPT]
 * @todo Add NMEA sentence: speed through water [$IIVHW]
 * @todo Add NMEA sentence: compass heading [$IIVHW, $IIHDM]
 * @todo Add NMEA sentence: trip and total milage [$IIVLW]
 */
static void process_nmea(struct information_t * current, const struct nmea_t * nmea)
{
	switch (nmea->type) {
		case NMEA_RMC:
			set_current_nmea_rmc(current, &nmea->sentence.rmc);
			break;
		default:
			/* NMEA type not supported by now or not interesting at all */
			break;
	}
}

static int prepare_date(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%04u-%02u-%02u;",
		curr->date.y, curr->date.m, curr->date.d);
}

static int prepare_time(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%02u-%02u-%02u;",
		curr->time.h, curr->time.m, curr->time.s);
}

static int prepare_latitude(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%02u-%02u,%1u%c;",
		curr->lat.d, curr->lat.m, (curr->lat.s.i * 100) / 60, curr->lat_dir);
}

static int prepare_longitude(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%03u-%02u,%1u%c;",
		curr->lon.d, curr->lon.m, (curr->lon.s.i * 100) / 60, curr->lon_dir);
}

static int prepare_course_over_ground(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u,%1u;",
		curr->course_over_ground.i, curr->course_over_ground.d / NMEA_FIX_DECIMALS);
}

static int prepare_speed_over_ground(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u,%1u;",
		curr->speed_over_ground.i, curr->speed_over_ground.d / NMEA_FIX_DECIMALS);
}

static int prepare_course_magnetic(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u,%1u;",
		curr->course_magnetic.i, curr->course_magnetic.d / NMEA_FIX_DECIMALS);
}

static int prepare_speed_through_water(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u,%1u;",
		curr->speed_through_water.i, curr->speed_through_water.d / NMEA_FIX_DECIMALS);
}

static int prepare_wind_speed(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u,%1u;",
		curr->wind_speed.i, curr->wind_speed.d / NMEA_FIX_DECIMALS);
}

static int prepare_wind_direction(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u;",
		curr->wind_direction.i);
}

static int prepare_pressure(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%u;",
		curr->pressure);
}

static int prepare_air_temperature(
		char * ptr,
		int len,
		const struct information_t * curr)
{
	return snprintf(ptr, len, "%d;",
		curr->air_temperature);
}

/**
 * @retval  0 Success, do write update
 * @retval -1 Unable to calculate update time, do not write
 * @retval -2 Last position update too long ago, do not write
 */
static int check_age_last_update(const struct logbook_data_t * data)
{
	long dt;

	dt = elapsed_ms_time(&data->current.last_update);
	if (dt < 0) {
		syslog(LOG_WARNING, "not writing log, cannot calculate update time");
		return -1;
	}
	if (dt > data->configuration.timeout_for_writing) {
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
static int check_position_change(const struct logbook_data_t * data)
{
	long ds = 0;

	if (data->configuration.min_meter_position_change <= 0)
		return 0; /* check disabled, always write log */

	ds = diff_position(&data->last_written_data, &data->current);

	if (ds < 0) {
		syslog(LOG_WARNING, "not writing log, cannot calculate position change");
		return -1;
	}
	if (ds < data->configuration.min_meter_position_change) {
		syslog(LOG_INFO, "not writing log, position has not changed significantly");
		return -2;
	}
	return 0;
}

/**
 * Write the log entry to either syslog or log file.
 * All data is separated by semi colons (aka CSV format).
 */
static void write_log(struct logbook_data_t * data)
{
	typedef int (*prepare_func_t)(char *, int, const struct information_t *);
	typedef int (*check_func_t)(const struct logbook_data_t *);

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
		if (CHECK[i](data))
			return;
	}

	/* prepare log entry */

	buf_len = (int)sizeof(buf);
	ptr = buf;
	memset(buf, 0, sizeof(buf));

	for (i = 0; i < sizeof(PREPARE) / sizeof(PREPARE[0]); ++i) {
		rc = PREPARE[i](ptr, buf_len, &data->current);
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

	if (data->configuration.filename_defined) {
		file = fopen(data->configuration.filename, "wt+");
		if (!file) {
			syslog(LOG_ERR, "cannot open logbook file '%s', error: %s", data->configuration.filename, strerror(errno));
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

	data->last_written_data = data->current;
}

static void process_timer(
		struct logbook_data_t * data,
		uint32_t id)
{
	if (!data->configuration.save_timer_defined)
		return;
	if (id != data->configuration.save_timer_id)
		return;
	write_log(data);
}

static int read_save_timer(
		struct logbook_config_t * configuration,
		const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	char * endptr;

	prop = proplist_find(properties, "save_timer_id");
	if (prop) {
		configuration->save_timer_id = strtoul(prop->value, &endptr, 0);
		if (strlen(prop->value) <= 0) {
			syslog(LOG_ERR, "invalid value in save_timer_id: '%s'", prop->value);
			return EXIT_FAILURE;
		}
		if (*endptr != '\0') {
			syslog(LOG_ERR, "invalid save_timer_id: '%s'", prop->key);
			return EXIT_FAILURE;
		}
		configuration->save_timer_defined = 1;
	} else {
		configuration->save_timer_defined = 0;
	}
	return EXIT_SUCCESS;
}

static int read_filename(
		struct logbook_config_t * configuration,
		const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	size_t len;

	prop = proplist_find(properties, "filename");
	if (!prop)
		return EXIT_FAILURE;

	len = strlen(prop->value);
	if (len > sizeof(configuration->filename) - 1) {
		return EXIT_FAILURE;
	}

	if (len > 0) {
		strncpy(configuration->filename, prop->value, sizeof(configuration->filename) - 1);
		configuration->filename_defined = 1;
	} else {
		configuration->filename_defined = 0;
	}

	return EXIT_SUCCESS;
}

static int read_timeout(
		struct logbook_config_t * configuration,
		const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "write_timeout");
	configuration->timeout_for_writing = 5; /* default value */
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
		configuration->timeout_for_writing = tmp;
	}
	configuration->timeout_for_writing *= 1000; /* unit: seconds */
	return EXIT_SUCCESS;
}

static int read_min_position_change(
		struct logbook_config_t * configuration,
		const struct property_list_t * properties)
{
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, "min_position_change");
	configuration->min_meter_position_change = 0; /* default value */
	if (prop != NULL) {
		long tmp = strtol(prop->value, &endptr, 0);
		if (strlen(prop->value) <= 0) {
			syslog(LOG_ERR, "invalid value in timeout: '%s'", prop->value);
			return EXIT_FAILURE;
		}
		if (*endptr != '\0') {
			syslog(LOG_ERR, "invalid value in timeout: '%s'", prop->value);
			return EXIT_FAILURE;
		}
		if (tmp < 0) {
			syslog(LOG_ERR, "invalid value for timeout: %ld, must be greater than zero", tmp);
			return EXIT_FAILURE;
		}
		configuration->min_meter_position_change = tmp;
	}
	return EXIT_SUCCESS;
}

static int init_proc(
		struct proc_config_t * configuration,
		const struct property_list_t * properties)
{
	struct logbook_data_t * data = NULL;

	if (!configuration)
		return EXIT_FAILURE;
	if (!properties)
		return EXIT_FAILURE;
	if (configuration->data != NULL)
		return EXIT_FAILURE;

	data = (struct logbook_data_t *)malloc(sizeof(struct logbook_data_t));
	configuration->data = data;
	init_data(data);

	if (read_save_timer(&data->configuration, properties) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (read_filename(&data->configuration, properties) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (read_timeout(&data->configuration, properties) != EXIT_SUCCESS)
		return EXIT_FAILURE;
	if (read_min_position_change(&data->configuration, properties) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	data->initialized = true;
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
	struct logbook_data_t * data;
	struct signalfd_siginfo signal_info;

	if (!config)
		return EXIT_FAILURE;

	data = (struct logbook_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	if (!data->initialized) {
		syslog(LOG_ERR, "uninitialized");
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
					process_nmea(&data->current, &msg.data.nmea);
					break;

				case MSG_TIMER:
					process_timer(data, msg.data.timer_id);
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
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = NULL,
};

