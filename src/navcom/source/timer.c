#include <navcom/source/timer.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/select.h>

static void init_data(struct timer_data_t * data)
{
	memset(data, 0, sizeof(struct timer_data_t));
}

static int configure(struct proc_config_t * config, const struct property_list_t * properties)
{
	struct timer_data_t * data = NULL;
	uint32_t t;
	const struct property_t * prop_id = NULL;
	const struct property_t * prop_period = NULL;
	char * endptr = NULL;

	if (config == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct timer_data_t *)malloc(sizeof(struct timer_data_t));
	config->data = data;
	init_data(data);

	prop_id = proplist_find(properties, "id");
	prop_period = proplist_find(properties, "period");

	if (!prop_id) {
		syslog(LOG_ERR, "no timer ID defined");
		return EXIT_FAILURE;
	}

	if (!prop_period) {
		syslog(LOG_ERR, "no timer period defined");
		return EXIT_FAILURE;
	}

	data->timer_id = strtoul(prop_id->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value in id: '%s'", prop_id->value);
		return EXIT_FAILURE;
	}

	t = strtoul(prop_period->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value in period: '%s'", prop_period->value);
		return EXIT_FAILURE;
	}
	data->tm_cfg.tv_sec = t / 1000;
	data->tm_cfg.tv_nsec = (t % 1000);
	data->tm_cfg.tv_nsec *= 1000000;

	data->initialized = 1;
	return EXIT_SUCCESS;
}

/**
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int clean(struct proc_config_t * config)
{
	if (config == NULL)
		return EXIT_FAILURE;

	if (config->data) {
		free(config->data);
		config->data = NULL;
	}

	return EXIT_SUCCESS;
}

/**
 * @todo Error handling: what if 'write' fails?
 */
static void send_message(
		const struct proc_config_t * config,
		struct message_t * msg)
{
	int rc;

	rc = write(config->wfd, msg, sizeof(struct message_t));
	if (rc < 0) {
		syslog(LOG_DEBUG, "unable to write to pipe: %s", strerror(errno));
	}
}

static int proc(struct proc_config_t * config)
{
	int rc;
	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;
	struct timer_data_t * data;
	struct message_t timer_message;

	if (!config)
		return EXIT_FAILURE;

	data = (struct timer_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	if (!data->initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	timer_message.type = MSG_TIMER;
	timer_message.data.timer_id = data->timer_id;

	while (!proc_request_terminate()) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max) fd_max = config->rfd;

		tm = data->tm_cfg;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timeout */
			send_message(config, &timer_message);
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				syslog(LOG_ERR, "unable to read from pipe: %s", strerror(errno));
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
	.name = "timer",
	.configure = configure,
	.func = proc,
	.clean = clean
};

