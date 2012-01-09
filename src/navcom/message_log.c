#include <navcom/message_log.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

enum { MAX_ERRORS = 10 };

static unsigned int cnt_error = 0;
static int prop_enable = 0;
static const char * prop_dst = NULL;

static void read_properties(const struct property_list_t * properties)
{
	prop_enable = proplist_contains(properties, "enable");
	prop_dst = proplist_value(properties, "dst");

	syslog(LOG_DEBUG, "enable:%d dst:'%s'", prop_enable, prop_dst);
}

static int log_message(const struct message_t * msg)
{
	int rc;
	char buf[NMEA_MAX_SENTENCE];
	FILE * file;

	memset(buf, 0, sizeof(buf));
	rc = nmea_write(buf, sizeof(buf), &msg->data.nmea);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to write NMEA data to buffer");
		return -1;
	}

	if (prop_dst == NULL) {
		syslog(LOG_DEBUG, "%s", buf);
		return 0;
	}

	file = fopen(prop_dst, "at");
	if (file == NULL) {
		perror("fopen");
		return -1;
	}
	fprintf(file, "%s\n", buf);
	fclose(file);

	return 0;
}

static int proc(const struct proc_config_t * config, const struct property_list_t * properties)
{
	fd_set rfds;
	struct message_t msg;
	int rc;

	read_properties(properties);

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
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
					if (prop_enable) {
						rc = log_message(&msg);
						if (rc < 0) {
							++cnt_error;
							if (cnt_error >= MAX_ERRORS) {
								syslog(LOG_ERR, "MAX_ERRORS (%u) reached, disable logging", MAX_ERRORS);
								prop_enable = 0;
							}
						}
					}
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

const struct proc_desc_t message_log = {
	"message_log",
	proc
};

