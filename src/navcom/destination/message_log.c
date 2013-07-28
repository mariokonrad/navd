#include <navcom/destination/message_log.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <navcom/property_read.h>
#include <common/macros.h>
#include <common/fileutil.h>
#include <sys/select.h>
#include <sys/signalfd.h>
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
	int fd_max;
	fd_set rfds;
	struct message_t msg;
	uint32_t cnt_error = 0;
	struct message_log_data_t * data;
	struct signalfd_siginfo signal_info;

	if (!config)
		return EXIT_FAILURE;

	data = (struct message_log_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

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

static void help(void)
{
	printf("\n");
	printf("message_log\n");
	printf("\n");
	printf("Logs NMEA messages.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  enable     : enable or disables the message log, does not take any arguments.\n");
	printf("  max_errors : number of errors the log will tolerate, the message log will disable\n");
	printf("               if this threshold is reached. As errors counts all write errors to\n");
	printf("               the configured destination or a corrupt NMEA message.\n");
	printf("  dst        : device/file to which the messages are being logged. If this is not\n");
	printf("               specified, messages are logged only to syslog.\n");
	printf("\n");
	printf("Example:\n");
	printf("  log : message_log { enable };\n");
	printf("\n");
	printf("  log : message_log { enable, dst:'/dev/'null' };\n");
	printf("\n");
	printf("  log : message_log { enable, dst:'some_logfile.txt' };\n");
	printf("\n");
}

const struct proc_desc_t message_log = {
	.name = "message_log",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = help,
};

