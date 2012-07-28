#include <navcom/timer.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

static int proc(const struct proc_config_t * config, const struct property_list_t * properties)
{
	/* TODO: timer wakeup and sending message */

	int rc;
	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	struct message_t timer_message;

	UNUSED_ARG(properties);

	timer_message.type = MSG_TIMER;
	timer_message.data.timer_id = 1; /* TODO: ID */

	while (!request_terminate) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max) fd_max = config->rfd;

		/* TODO: configuration */
		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timerout */
			rc = write(config->wfd, &timer_message, sizeof(timer_message));
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
	NULL,
	proc
};



