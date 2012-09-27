#include <navcom/destination/message_log.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

enum { MAX_ERRORS = 10 }; /* TODO: configuration/properties, module providing default value */

struct msg_log_property_t {
	int enable;
	const char * dst;
};

static struct msg_log_property_t prop;

static int log_message(const struct message_t * msg, const struct msg_log_property_t * prop)
{
	int rc;
	char buf[NMEA_MAX_SENTENCE];
	FILE * file;

	if (msg == NULL) {
		return -1;
	}
	if (prop == NULL) {
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	rc = nmea_write(buf, sizeof(buf), &msg->data.nmea);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to write NMEA data to buffer");
		return -1;
	}

	if (prop->dst == NULL) {
		syslog(LOG_DEBUG, "%s", buf);
		return 0;
	}

	file = fopen(prop->dst, "at");
	if (file == NULL) {
		syslog(LOG_ERR, "unable to open destination '%s': %s", prop->dst, strerror(errno));
		return -1;
	}
	fprintf(file, "%s\n", buf);
	fclose(file);

	return 0;
}

static int is_writable(const char * path)
{
	int rc;
	struct stat s;

	if (!path) return 0;
	if (strlen(path)) return 0;

	rc = stat(path, &s);
	if (rc < 0) return 0;

	if (!S_ISREG(s.st_mode)) return 0;
	if (!(s.st_mode & S_IWUSR)) return 0;

	return 1;
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	const struct property_t * prop_dst = NULL;

	UNUSED_ARG(config);

	memset(&prop, 0, sizeof(struct msg_log_property_t));

	prop.enable = proplist_contains(properties, "enable");

	prop_dst = proplist_find(properties, "dst");
	if (prop_dst) {
		if (is_writable(prop_dst->value)) {
			prop.dst = prop_dst->value;
		} else {
			syslog(LOG_ERR, "%s:destination not writable, logging to syslog only", config->cfg->name);
		}
	} else {
		syslog(LOG_ERR, "%s:no destination specified, logging to syslog only", config->cfg->name);
	}

	syslog(LOG_DEBUG, "%s:enable:%d dst:'%s'", config->cfg->name, prop.enable, prop.dst);
	return EXIT_SUCCESS;
}

static int proc(const struct proc_config_t * config)
{
	int rc;
	fd_set rfds;
	struct message_t msg;
	unsigned int cnt_error = 0;

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
					if (prop.enable) {
						rc = log_message(&msg, &prop);
						if (rc < 0) {
							++cnt_error;
							if (cnt_error >= MAX_ERRORS) {
								syslog(LOG_ERR, "MAX_ERRORS (%u) reached, disable logging", MAX_ERRORS);
								prop.enable = 0;
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
	.name = "message_log",
	.configure = configure,
	.func = proc
};

