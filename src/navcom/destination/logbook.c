#include <navcom/destination/logbook.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <sys/select.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <nmea/nmea.h>

static int initialized = 0;

static struct logbook_config_t {
	uint32_t save_timer_id;
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

static void process_timer(uint32_t id)
{
	UNUSED_ARG(id);

	/* TODO: invoke actions according to configured timer triggers */
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	UNUSED_ARG(config);
	UNUSED_ARG(properties);

	/* TODO: initialization: save_timer_id */

	return 0;
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

