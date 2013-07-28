#include <navcom/source/timer.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/signalfd.h>

static void init_data(struct timer_data_t * data)
{
	memset(data, 0, sizeof(struct timer_data_t));
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
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
	int fd_max;
	struct message_t msg;
	struct timespec tm;
	struct timer_data_t * data;
	struct message_t timer_message;
	struct signalfd_siginfo signal_info;

	if (!config)
		return EXIT_FAILURE;

	data = (struct timer_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	if (!data->initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	/* setup signal handling */
	timer_message.type = MSG_TIMER;
	timer_message.data.timer_id = data->timer_id;

	while (1) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max)
			fd_max = config->rfd;
		FD_SET(config->signal_fd, &rfds);
		if (config->signal_fd > fd_max)
			fd_max = config->signal_fd;

		tm = data->tm_cfg;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, NULL);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) /* timeout */
			if (message_write(config->wfd, &timer_message) != EXIT_SUCCESS)
				return EXIT_FAILURE;

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
				default:
					break;
			}
			continue;
		}
	}

	return EXIT_SUCCESS;
}

static void help(void)
{
	printf("\n");
	printf("timer\n");
	printf("\n");
	printf("Sends timer messages periodically.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  id     : unsigned numerical identifier\n");
	printf("  period : time period in msec in which the message will be sent.\n");
	printf("\n");
	printf("Example:\n");
	printf("  logtimer : timer { id:1, period:5000 };\n");
	printf("\n");
}

const struct proc_desc_t timer = {
	.name = "timer",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = help,
};

