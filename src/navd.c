#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <common/macros.h>
#include <message.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <device/serial.h>
#include <config/config.h>

static const char * OPTIONS_SHORT = "hdc:";

static const struct option OPTIONS_LONG[] =
{
	{ "help",        no_argument,       0, 'h' },
	{ "daemon",      no_argument,       0, 'd' },
	{ "config",      required_argument, 0, 'c' },
	{ "dump-config", no_argument,       0, 0   },
};

static struct {
	int daemonize;
	int config;
	int dump_config;
	char config_filename[PATH_MAX+1];
} option;

static volatile int request_terminate = 0;
static sigset_t signal_mask;

typedef struct proc_config_t {
	int pid; /* process id */
	int rfd; /* pipe file descriptor to read */
	int wfd; /* pipe file descriptor to write */
} proc_config_t;

static proc_config_t * proc_cfg = NULL;
static size_t proc_cfg_base_src = 0;
static size_t proc_cfg_base_dst = 0;

static void usage(const char * name) /* {{{ */
{
	printf("\n");
	printf("usage: %s [options]\n", name);
	printf("\n");
	printf("Options:\n");
	printf("  -h      | --help        : help information\n");
	printf("  -d      | --daemon      : daemonize process\n");
	printf("  -c file | --config file : configuration file\n");
	printf("  --dump-config           : dumps the configuration and exit\n");
	printf("\n");
} /* }}} */

static int parse_options(int argc, char ** argv) /* {{{ */
{
	int rc;
	int index;

	memset(&option, 0, sizeof(option));
	while (optind < argc) {
		rc = getopt_long(argc, argv, OPTIONS_SHORT, OPTIONS_LONG, &index);
		if (rc == -1) {
			return -1;
		}
		switch (rc) {
			case 'h':
				usage(argv[0]);
				return -1;
			case 'd':
				option.daemonize = 1;
				break;
			case 'c':
				option.config = 1;
				strncpy(option.config_filename, optarg, sizeof(option.config_filename)-1);
				break;
			case 0:
				switch (index) {
					case 3:
						option.dump_config = 1;
						break;
				}
				break;
			default:
				break;
		}
	}
	if (optind < argc) {
		printf("error: unknown parameters\n");
		usage(argv[0]);
		return -1;
	}
	return 0;
} /* }}} */

static void prepare_proc_configs(size_t num_sources, size_t num_destinations) /* {{{ */
{
	size_t i;
	size_t num = num_sources + num_destinations;

	proc_cfg = malloc(sizeof(struct proc_config_t) * num);
	for (i = 0; i < num; ++i) {
		proc_cfg[i].pid = -1;
		proc_cfg[i].rfd = -1;
		proc_cfg[i].wfd = -1;
	}
	proc_cfg_base_src = 0;
	proc_cfg_base_dst = num_sources;
} /* }}} */

static void destroy_proc_configs(void) /* {{{ */
{
	if (proc_cfg) {
		free(proc_cfg);
		proc_cfg = NULL;
	}
} /* }}} */

static void handle_signal(int sig) /* {{{ */
{
	printf("%s:%d: pid:%d sig:%d\n", __FILE__, __LINE__, getpid(), sig);
	request_terminate = 1;
} /* }}} */

static void daemonize(void) /* {{{ */
{
	int rc;

	rc = fork();
	if (rc < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (rc > 0) {
		exit(rc);
	}
} /* }}} */

static int start_proc(proc_config_t * proc, void (*func)(const proc_config_t *, const struct property_list_t *), const struct property_list_t * prop) /* {{{ */
{
	int rc;
	int rfd[2]; /* hub -> proc */
	int wfd[2]; /* proc -> hub */

	if (proc == NULL) return -1;
	if (func == NULL) return -1;

	rc = pipe(rfd);
	if (rc < 0) {
		perror("pipe");
		return -1;
	}
	rc = pipe(wfd);
	if (rc < 0) {
		close(rfd[0]);
		close(rfd[1]);
		perror("pipe");
		return -1;
	}

	rc = fork();
	if (rc < 0) {
		close(rfd[0]);
		close(rfd[1]);
		close(wfd[0]);
		close(wfd[1]);
		perror("fork");
		return -1;
	}

	if (rc == 0) {
		/* child code */
		proc->pid = getpid();
		proc->rfd = rfd[0];
		proc->wfd = wfd[1];
		close(rfd[1]);
		close(wfd[0]);
		func(proc, prop); /* child code, will not return */
		exit(EXIT_SUCCESS);
	} else {
		proc->pid = rc;
		proc->rfd = wfd[0];
		proc->wfd = rfd[1];
		close(rfd[0]);
		close(wfd[1]);
	}
	return rc;
} /* }}} */

static void dump_config(const struct config_t * config) /* {{{ */
{
	size_t i;
	size_t j;

	if (config == NULL) return;

	printf("===== SOURCES ========================\n");
	for (i = 0; i < config->num_sources; ++i) {
		struct source_t * p = &config->sources[i];
		printf("  %s : %s\n", p->name, p->type);
		for (j = 0; j < p->properties.num; ++j) {
			const struct property_t * prop = &p->properties.data[j];
			printf("\t%s = %s\n", prop->key, prop->value);
		}
	}
	printf("===== DESTINATIONS ===================\n");
	for (i = 0; i < config->num_destinations; ++i) {
		struct destination_t * p = &config->destinations[i];
		printf("  %s : %s\n", p->name, p->type);
		for (j = 0; j < p->properties.num; ++j) {
			const struct property_t * prop = &p->properties.data[j];
			printf("\t%s = %s\n", prop->key, prop->value);
		}
	}
	printf("===== FILTERS ========================\n");
	for (i = 0; i < config->num_filters; ++i) {
		struct filter_t * p = &config->filters[i];
		printf("  %s : %s\n", p->name, p->type);
		for (j = 0; j < p->properties.num; ++j) {
			const struct property_t * prop = &p->properties.data[j];
			printf("\t%s = %s\n", prop->key, prop->value);
		}
	}
	printf("===== ROUTES =========================\n");
	for (i = 0; i < config->num_routes; ++i) {
		struct route_t * p = &config->routes[i];
		printf("  %s --[%s]--> %s    (%p, %p, %p)\n", p->name_source, p->name_filter, p->name_destination,
			p->source, p->filter, p->destination);
	}
} /* }}} */

static void gps_device_serial_demo(const proc_config_t * config, const struct property_list_t * properties) /* <source> {{{ */
{
	int rc;

	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	struct device_t device;
	const struct device_operations_t * ops = NULL;

	struct nmea_t nmea;
	char buf[NMEA_MAX_SENTENCE + 1];
	int buf_index;
	char c;

	struct serial_config_t serial_config = {
		"/dev/ttyUSB0"
	};

	UNUSED_ARG(properties);

	ops = &serial_device_operations;
	device_init(&device);
	nmea_init(&nmea);
	memset(buf, 0, sizeof(buf));
	buf_index = 0;

	rc = ops->open(&device, (const struct device_config_t *)&serial_config);
	if (rc < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	while (!request_terminate) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max) fd_max = config->rfd;
		FD_SET(device.fd, &rfds);
		if (device.fd > fd_max) fd_max = device.fd;

		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (FD_ISSET(device.fd, &rfds)) {
			rc = ops->read(&device, &c, sizeof(c));
			if (rc < 0) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (rc != sizeof(c)) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			switch (c) {
				case '\r':
					break;
				case '\n':
					rc = nmea_read(&nmea, buf);
					if (rc == 0) {
						printf("OK : [%s]\n", buf);
						/* TODO: send message */
					} else if (rc == 1) {
						printf("[%s] : UNKNOWN SENTENCE\n", buf);
					} else if (rc == -2) {
						printf("[%s] : CHECKSUM ERROR\n", buf);
					} else {
						fprintf(stderr, "parameter error\n");
						exit(EXIT_FAILURE);
					}
					buf_index = 0;
					buf[buf_index] = 0;
					break;
				default:
					if (buf_index < NMEA_MAX_SENTENCE) {
						buf[buf_index++] = c;
					} else {
						fprintf(stderr, "sentence too long, discarding\n");
						buf_index = 0;
					}
					buf[buf_index] = 0;
					break;
			}
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "error: cannot read message\n");
				continue;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							rc = ops->close(&device);
							if (rc < 0) {
								perror("close");
								exit(EXIT_FAILURE);
							}
							exit(EXIT_SUCCESS);
						default:
							break;
					}
				default:
					break;
			}
			continue;
		}
	}
} /* }}} */

static void gps_simulator(const proc_config_t * config, const struct property_list_t * properties) /* <source> {{{ */
{
	fd_set rfds;
	struct message_t msg;
	struct message_t sim_message;
	int rc;
	struct timespec tm;

	struct nmea_rmc_t * rmc;
	char buf[NMEA_MAX_SENTENCE];

	UNUSED_ARG(properties);

	sim_message.type = MSG_NMEA;
	sim_message.data.nmea.type = NMEA_RMC;
	rmc = &sim_message.data.nmea.sentence.rmc;
	rmc->time.h = 20;
	rmc->time.m = 15;
	rmc->time.s = 30;
	rmc->time.ms = 0;
	rmc->status = NMEA_STATUS_OK;
	rmc->lat.d = 47;
	rmc->lat.m = 8;
	rmc->lat.s.i = 0;
	rmc->lat.s.d = 0;
	rmc->lat_dir = NMEA_NORTH;
	rmc->lon.d = 3;
	rmc->lon.m = 20;
	rmc->lon.s.i = 0;
	rmc->lon.s.d = 0;
	rmc->lon_dir = NMEA_EAST;
	rmc->sog.i = 0;
	rmc->sog.d = 0;
	rmc->head.i = 0;
	rmc->head.d = 0;
	rmc->date.y = 0;
	rmc->date.m = 1;
	rmc->date.d = 1;
	rmc->m.i = 0;
	rmc->m.d = 0;
	rmc->m_dir = NMEA_WEST;
	rmc->sig_integrity = NMEA_SIG_INT_SIMULATED;

	rc = nmea_write(buf, sizeof(buf), &sim_message.data.nmea);
	if (rc < 0) {
		fprintf(stderr, "%s:%d: invalid RMC data, rc=%d\n", __FILE__, __LINE__, rc);
		exit(EXIT_FAILURE);
	}

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timeout */
			rc = write(config->wfd, &sim_message, sizeof(sim_message));
			if (rc < 0) {
				perror("write");
				fprintf(stderr, "%s:%d: wfd=%d\n", __FILE__, __LINE__, config->wfd);
			}
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "error: cannot read message\n");
				continue;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							exit(EXIT_SUCCESS);
						default:
							break;
					}
				default:
					break;
			}
			continue;
		}
	}
} /* }}} */

static void message_log(const proc_config_t * config, const struct property_list_t * properties) /* <destination> {{{ */
{
	fd_set rfds;
	struct message_t msg;
	int rc;
	char buf[NMEA_MAX_SENTENCE];

	/* TODO: properties */
	UNUSED_ARG(properties);

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			rc = read(config->rfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "error: cannot read message\n");
				continue;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							exit(EXIT_SUCCESS);
						default:
							break;
					}
					break;

				case MSG_NMEA:
					memset(buf, 0, sizeof(buf));
					rc = nmea_write(buf, sizeof(buf), &msg.data.nmea);
					if (rc < 0) {
						fprintf(stderr, "%s:%d: error while writing NMEA data to buffer\n", __FILE__, __LINE__);
						continue;
					}
					printf("%s:%d: received message: [%s]\n", __FILE__, __LINE__, buf);
					break;

				default:
					printf("%s:%d: log  : %08x\n", __FILE__, __LINE__, msg.type);
					break;
			}
			continue;
		}
	}
} /* }}} */

static int filter_dummy(struct message_t * out, const struct message_t * in) /* <filter> {{{ */
{
	if (out == NULL) return -1;
	if (in == NULL) return -1;

	memcpy(out, in, sizeof(struct message_t));
	return 0;
} /* }}} */

static int read_configuration(struct config_t * config) /* {{{ */
{
	int rc;

	if (option.config != 1) {
		printf("error: configuration file name not defined.\n");
		return -1;
	}
	if (strlen(option.config_filename) <= 0) {
		printf("error: invalid config file name.'\n");
		return -1;
	}
	rc = config_parse_file(option.config_filename, config);
	if (rc < 0) {
		printf("error: unable to read config file.\n");
		return -1;
	}

	return 0;
} /* }}} */

static int send_terminate(const struct proc_config_t * proc) /* {{{ */
{
	int rc;
	struct message_t msg;

	if (!proc || proc->pid <= 0) return 0;
	memset(&msg, 0, sizeof(msg));
	msg.type = MSG_SYSTEM;
	msg.data.system = SYSTEM_TERMINATE;
	rc = write(proc->wfd, &msg, sizeof(msg));
	if (rc < 0) {
		perror("write");
		return -1;
	}
	return 0;
} /* }}} */

static int start_destinations(const struct config_t * config) /* {{{ */
{
	size_t i;
	int rc;

	for (i = 0; i < config->num_destinations; ++i) {
		const struct destination_t * ptr = &config->destinations[i];
		struct proc_config_t * cfg = &proc_cfg[i + proc_cfg_base_dst];
		printf("start destination '%s' (type: '%s')\n", ptr->name, ptr->type);
		if (!strcmp(ptr->type, "message_log")) {
			rc = start_proc(cfg, message_log, &ptr->properties);
			if (rc < 0) {
				return -1;
			}
		} else {
			fprintf(stderr, "%s:%d: error: unknown destination\n", __FILE__, __LINE__);
			return -1;
		}
	}
	return 0;
} /* }}} */

static int start_sources(const struct config_t * config) /* {{{ */
{
	size_t i;
	int rc;

	for (i = 0; i < config->num_sources; ++i) {
		const struct source_t * ptr = &config->sources[i];
		struct proc_config_t * cfg = &proc_cfg[i + proc_cfg_base_src];
		printf("start source '%s' (type: '%s')\n", ptr->name, ptr->type);
		if (!strcmp(ptr->type, "gps_sim")) {
			rc = start_proc(cfg, gps_simulator, &ptr->properties);
			if (rc < 0) {
				exit(EXIT_FAILURE);
			}
		} else if (!strcmp(ptr->type, "gps_serial")) {
			rc = start_proc(cfg, gps_device_serial_demo, &ptr->properties);
			if (rc < 0) {
				exit(EXIT_FAILURE);
			}
		} else {
			fprintf(stderr, "%s:%d: error: unknown destination\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
		}
	}
	return 0;
} /* }}} */

static int route_msg(size_t src_index, const struct config_t * config, const struct message_t * msg)
{
	UNUSED_ARG(src_index);
	UNUSED_ARG(config);
	UNUSED_ARG(msg);

	/* TODO */
#if 0
			switch (msg.type) {
				default:
					rc = write(cfg_log.wfd, &msg, sizeof(msg));
					if (rc < 0) {
						perror("write");
					}
					break;
			}
#endif

	return 0;
}

int main(int argc, char ** argv) /* {{{ */
{
	size_t i;

	struct message_t msg;
	int rc;
	fd_set rfds;
	int fd_max;
	int num_msg;
	int fd;

	sigset_t mask;
	struct sigaction act;

	struct config_t config;

	/* register known types (sources, destinations, filters) */
	config_register_source("gps_sim");
	config_register_source("gps_serial");
	config_register_destination("message_log");
	config_register_filter("message_filter");
	config_register_filter("filter_dummy");

	if (parse_options(argc, argv) < 0) return EXIT_FAILURE;
	if (read_configuration(&config) < 0) return EXIT_FAILURE;

	/* TODO: check config (no duplicates, etc.) */

	/* dump information */
	if (option.dump_config) {
		dump_config(&config);
		config_free(&config);
		exit(EXIT_SUCCESS);
	}

	/* daemonize process */
	if (option.daemonize) {
		daemonize();
	}

	/* set up signal handling (active also for all subprocesses, if masks are global) */
	memset(&act, 0, sizeof(act));
	act.sa_handler = handle_signal;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	if (sigprocmask(SIG_BLOCK, &mask, &signal_mask) < 0) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}

	/* setup processes */
	prepare_proc_configs(config.num_sources, config.num_destinations);

	/* start subprocesses */
	if (start_destinations(&config) < 0) return EXIT_FAILURE;
	if (start_sources(&config) < 0) return EXIT_FAILURE;

	/* main / hub process */
	num_msg = 5; /* TODO */
	for (; !request_terminate;) {
		FD_ZERO(&rfds);
		fd_max = 0;
		for (i = 0; i < config.num_sources + config.num_destinations; ++i) {
			fd = proc_cfg[i].rfd;
			if (fd <= 0) continue;
			FD_SET(fd, &rfds);
			if (fd > fd_max) {
				fd_max = fd;
			}
		}

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (request_terminate) {
			break;
		} else if (rc == 0) {
			continue;
		}

		for (i = 0; i < config.num_sources + config.num_destinations; ++i) {
			fd = proc_cfg[i].rfd;
			if (fd <= 0) continue;
			if (!FD_ISSET(fd, &rfds)) continue;

			rc = read(fd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "%s:%d: error: cannot read message\n", __FILE__, __LINE__);
				continue;
			}

			if ((i >= proc_cfg_base_src) && (i < proc_cfg_base_dst)) {
				if (route_msg(i, &config, &msg) < 0) {
					fprintf(stderr, "%s:%d: error: route: %08x\n", __FILE__, __LINE__, msg.type);
				} else {
					printf("%s:%d: route: %08x\n", __FILE__, __LINE__, msg.type);
				}
			} else {
				fprintf(stderr, "%s:%d: error: messages from destinations not supported yet\n", __FILE__, __LINE__);
			}
		}

		/* TODO:TEMP: terminate after num_msg */
		--num_msg;
		if (num_msg <= 0) {
			break;
		}
	}

	/* request subprocesses to terminate, gracefully, and wait for their termination */
	for (i = 0; i < config.num_sources + config.num_destinations; ++i) {
		send_terminate(&proc_cfg[i]);
	}
	for (i = 0; i < config.num_sources + config.num_destinations; ++i) {
		waitpid(proc_cfg[i].pid, NULL, 0);
	}

	destroy_proc_configs();

	return EXIT_SUCCESS;
} /* }}} */

