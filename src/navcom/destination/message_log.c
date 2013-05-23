#include <navcom/destination/message_log.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <navcom/property_read.h>
#include <common/macros.h>
#include <common/fileutil.h>
#include <sys/select.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

struct message_log_data_t {
	int enable;
	char dst[PATH_MAX];
	uint32_t max_errors;
};

static int log_message(
		const struct message_t * msg,
		const struct message_log_data_t * data)
{
	int rc;
	char buf[NMEA_MAX_SENTENCE];
	FILE * file;

	if (msg == NULL)
		return EXIT_FAILURE;
	if (data == NULL)
		return EXIT_FAILURE;

	memset(buf, 0, sizeof(buf));
	rc = nmea_write(buf, sizeof(buf), &msg->data.nmea);
	if (rc < 0) {
		syslog(LOG_ERR, "unable to write NMEA data to buffer");
		return EXIT_FAILURE;
	}

	/* only to syslog if no destination is configured */
	if (strlen(data->dst) == 0) {
		syslog(LOG_DEBUG, "%s", buf);
		return EXIT_SUCCESS;
	}

	file = fopen(data->dst, "at");
	if (file == NULL) {
		syslog(LOG_ERR, "unable to open destination '%s': %s", data->dst, strerror(errno));
		return EXIT_FAILURE;
	}
	fprintf(file, "%s\n", buf);
	fclose(file);

	return EXIT_SUCCESS;
}

static void init_data(struct message_log_data_t * data)
{
	memset(data, 0, sizeof(struct message_log_data_t));
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	const struct property_t * prop_dst = NULL;
	struct message_log_data_t * data = NULL;

	if (config == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct message_log_data_t *)malloc(sizeof(struct message_log_data_t));
	config->data = data;
	init_data(data);

	data->enable = proplist_contains(properties, "enable");

	data->max_errors = 10; /* default value */
	if (property_read_uint32(properties, "max_errors",  &data->max_errors) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	prop_dst = proplist_find(properties, "dst");
	if (prop_dst) {
		if (file_is_writable(prop_dst->value)) {
			strncpy(data->dst, prop_dst->value, sizeof(data->dst));
		} else {
			syslog(LOG_ERR, "%s:destination not writable, logging to syslog only", config->cfg->name);
		}
	} else {
		syslog(LOG_ERR, "%s:no destination specified, logging to syslog only", config->cfg->name);
	}

	syslog(LOG_DEBUG, "%s:enable:%d dst:'%s'", config->cfg->name, data->enable, data->dst);
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
	fd_set rfds;
	struct message_t msg;
	uint32_t cnt_error = 0;
	struct message_log_data_t * data;

	if (!config)
		return EXIT_FAILURE;

	data = (struct message_log_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	while (!proc_request_terminate()) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
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
					if (data->enable) {
						rc = log_message(&msg, data);
						if (rc < 0) {
							++cnt_error;
							if (cnt_error >= data->max_errors) {
								syslog(LOG_ERR, "max_errors (%u) reached, disable logging", data->max_errors);
								data->enable = 0;
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
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = NULL,
};

