#include <navcom/timer.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

static int initialized = 0;
static struct timespec tm_cfg;
static struct message_t timer_message;

static int prop(struct proc_config_t * config, const struct property_list_t * properties)
{
	uint32_t t;

	UNUSED_ARG(config);

	if (!proplist_contains(properties, "id")) {
		syslog(LOG_ERR, "no timer ID defined");
		return EXIT_FAILURE;
	}

	if (!proplist_contains(properties, "period")) {
		syslog(LOG_ERR, "no timer period defined");
		return EXIT_FAILURE;
	}

	timer_message.data.timer_id = strtoul(proplist_value(properties, "id"), NULL, 0);

	t = strtoul(proplist_value(properties, "period"), NULL, 0);
	tm_cfg.tv_sec = t / 1000;
	tm_cfg.tv_nsec = (t % 1000);
	tm_cfg.tv_nsec *= 1000000;

	timer_message.type = MSG_TIMER;

	initialized = 1;
	return EXIT_SUCCESS;
}

static void send_message(const struct proc_config_t * config)
{
	int rc;

	rc = write(config->wfd, &timer_message, sizeof(timer_message));
	if (rc < 0) {
		perror("write");
		syslog(LOG_DEBUG, "wfd=%d", config->wfd);
	}
}

static int proc(const struct proc_config_t * config, const struct property_list_t * properties)
{
	int rc;
	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	UNUSED_ARG(properties);

	if (!initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	while (!request_terminate) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max) fd_max = config->rfd;

		tm = tm_cfg;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timerout */
			send_message(config);
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
							if (rc < 0) {
								perror("close");
								return EXIT_FAILURE;
							}
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

const struct proc_desc_t timer = {
	"timer",
	prop,
	proc
};

