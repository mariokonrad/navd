#include <global_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <libgen.h>
#include <unistd.h>

#include <common/macros.h>
#include <config/config.h>
#include <navcom/message.h>
#include <navcom/proc_list.h>
#include <navcom/filter_list.h>

#ifdef ENABLE_FILTER_LUA
	#include <navcom/filter/filter_lua.h>
#endif

#ifdef ENABLE_SOURCE_GPSSERIAL
	#include <navcom/source/gps_serial.h>
#endif

#ifdef ENABLE_SOURCE_GPSSIMULATOR
	#include <navcom/source/gps_simulator.h>
#endif

#ifdef ENABLE_DESTINATION_LUA
	#include <navcom/destination/dst_lua.h>
#endif

#ifdef ENABLE_SOURCE_LUA
	#include <navcom/source/src_lua.h>
#endif

#include <navcom/filter/filter_null.h>
#include <navcom/filter/filter_nmea.h>
#include <navcom/destination/message_log.h>
#include <navcom/destination/nmea_serial.h>
#include <navcom/destination/logbook.h>
#include <navcom/source/timer.h>

#if !defined(max)
	#define max(a, b)  ((a) > (b) ? (a) : (b))
#endif

#if !defined(min)
	#define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

/**
 * @todo Documentation
 */
enum options_t {
	 OPTION_HELP        = 'h'
	,OPTION_VERSION     = 'v'
	,OPTION_DEAMON      = 'd'
	,OPTION_CONFIG      = 'c'
	,OPTION_LIST        = 1000
	,OPTION_DUMP_CONFIG
	,OPTION_MAX_MSG
	,OPTION_LOG
};

static const char * OPTIONS_SHORT = "hvdc:";

static const struct option OPTIONS_LONG[] =
{
	{ "help",        no_argument,       0, OPTION_HELP        },
	{ "version",     no_argument,       0, OPTION_VERSION     },
	{ "daemon",      no_argument,       0, OPTION_DEAMON      },
	{ "config",      required_argument, 0, OPTION_CONFIG      },
	{ "list",        no_argument,       0, OPTION_LIST        },
	{ "dump-config", no_argument,       0, OPTION_DUMP_CONFIG },
	{ "max-msg",     required_argument, 0, OPTION_MAX_MSG     },
	{ "log",         required_argument, 0, OPTION_LOG         },
};

/**
 * This structure contains all possible options the program provides.
 */
static struct {
	int daemonize;
	int config;
	int dump_config;
	int list;
	unsigned int max_msg;
	int log_mask;
	char config_filename[PATH_MAX+1];
} option;


/**
 * Structure to hold all runtime information about a route
 * for messages from sources through filters to destinations.
 */
struct msg_route_t {
	/**
	 * Source of a message. This information is mandatory.
	 */
	struct proc_config_t * source;

	/**
	 * Destination of a message. This information is mandatory.
	 */
	struct proc_config_t * destination;

	/**
	 * Filter for the message. This information is optional.
	 * If this is NULL, no filter is applied to the message
	 * and the original message is routed to the destination.
	 */
	const struct filter_desc_t const * filter;

	/**
	 * Configuration (properties) of the filter. This may be NULL.
	 */
	const struct property_list_t const * filter_cfg;

	/**
	 * Runtime information of the filter. This context
	 * may hold any information the filter sees fit and is
	 * unique to the route. This means the same filter may
	 * be applied to different routes, the context however
	 * is unique to the route.
	 */
	struct filter_context_t filter_ctx;
};

/**
 * Array containing all procedures (sources and destinations).
 *
 * Since they have the same interface for administration, they
 * are kept in one array. This way it is easier to send them
 * all the same system message, e.g. termination message.
 *
 * This array is organized the way that all sources are
 * in consecutive order, followed by all destinations.
 * The starting indices are defined by proc_cfg_base_src
 * and proc_cfg_base_dst.
 */
static struct proc_config_t * proc_cfg = NULL;

/**
 * Index within the array proc_cfg from which the sources are
 * being stored.
 */
static size_t proc_cfg_base_src = 0;

/**
 * Index within the array proc_cfg from which the destinations are
 * being stored.
 */
static size_t proc_cfg_base_dst = 0;

/**
 * Array of runtime information of all configured routes.
 */
static struct msg_route_t * msg_routes = NULL;

/**
 * List of all available sources.
 */
static struct proc_desc_list_t desc_sources;

/**
 * List of all available destinations.
 */
static struct proc_desc_list_t desc_destinations;

/**
 * List of all available filters.
 */
static struct filter_desc_list_t desc_filters;

static void print_version(FILE * file)
{
	fprintf(file, "%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

static void print_config(FILE * file)
{
#if defined(ENABLE_SOURCE_GPSSERIAL)
	fprintf(file, " gpsserial ");
#endif

#if defined(ENABLE_SOURCE_GPSSIMULATOR)
	fprintf(file, " gpssimulator ");
#endif

#if defined(ENABLE_SOURCE_GPSD)
	fprintf(file, " gpsd ");
#endif

#if defined(ENABLE_FILTER_LUA)
	fprintf(file, " filter_lua(%s) ", filter_lua_release());
#endif

#if defined(ENABLE_DESTINATION_LUA)
	fprintf(file, " dst_lua(%s) ", dst_lua_release());
#endif

	fprintf(file, "\n");
}

static void usage(FILE * file, const char * name) /* {{{ */
{
	fprintf(file, "\n");
	fprintf(file, "usage: %s [options]\n", name);
	fprintf(file, "\n");
	fprintf(file, "Version: ");
	print_version(file);
	fprintf(file, "\n");
	fprintf(file, "Configured: ");
	print_config(file);
	fprintf(file, "\n");
	fprintf(file, "Options:\n");
	fprintf(file, "  -h      | --help        : help information\n");
	fprintf(file, "  -v      | --version     : version information\n");
	fprintf(file, "  -d      | --daemon      : daemonize process\n");
	fprintf(file, "  -c file | --config file : configuration file\n");
	fprintf(file, "  --list                  : lists all sources, destinations and filters\n");
	fprintf(file, "  --dump-config           : dumps the configuration and exit\n");
	fprintf(file, "  --max-msg n             : routes n number of messages before terminating\n");
	fprintf(file, "  --log n                 : defines log level on syslog (0..7)\n");
	fprintf(file, "\n");
} /* }}} */

static int parse_options(int argc, char ** argv) /* {{{ */
{
	int rc;
	int index;
	char * endptr = NULL;

	/* default values */
	memset(&option, 0, sizeof(option));
	option.log_mask = LOG_DEBUG;

	while (1) {
		rc = getopt_long(argc, argv, OPTIONS_SHORT, OPTIONS_LONG, &index);
		if (rc == -1) {
			break;
		}
		switch (rc) {
			case OPTION_HELP:
				usage(stdout, argv[0]);
				return -1;
			case OPTION_VERSION:
				print_version(stdout);
				return -1;
			case OPTION_DEAMON:
				option.daemonize = 1;
				break;
			case OPTION_CONFIG:
				option.config = 1;
				strncpy(option.config_filename, optarg, sizeof(option.config_filename)-1);
				break;
			case OPTION_LIST:
				option.list = 1;
				break;
			case OPTION_DUMP_CONFIG:
				option.dump_config = 1;
				break;
			case OPTION_MAX_MSG:
				option.max_msg = strtoul(optarg, &endptr, 0);
				if (*endptr != '\0') {
					syslog(LOG_ERR, "invalid value for parameter '%s': '%s'", OPTIONS_LONG[index].name, optarg);
					return -1;
				}
				break;
			case OPTION_LOG:
				option.log_mask = strtoul(optarg, &endptr, 0);
				if (*endptr != '\0') {
					syslog(LOG_ERR, "invalid value for parameter '%s': '%s'", OPTIONS_LONG[index].name, optarg);
					return -1;
				}
				option.log_mask = max(LOG_EMERG, option.log_mask);
				option.log_mask = min(LOG_DEBUG, option.log_mask);
				break;
			default:
				break;
		}
	}
	if (optind < argc) {
		syslog(LOG_ERR, "unknown parameters");
		usage(stdout, argv[0]);
		return -1;
	}
	return 0;
} /* }}} */

static void destroy_proc_configs(void) /* {{{ */
{
	if (proc_cfg) {
		free(proc_cfg);
		proc_cfg = NULL;
	}
} /* }}} */

static void prepare_proc_configs(const struct config_t * config) /* {{{ */
{
	size_t i;
	size_t num = config->num_sources + config->num_destinations;
	struct proc_config_t * proc;

	destroy_proc_configs();
	proc_cfg = malloc(sizeof(struct proc_config_t) * num);
	for (i = 0; i < num; ++i) {
		proc_config_init(&proc_cfg[i]);
	}
	proc_cfg_base_src = 0;
	proc_cfg_base_dst = config->num_sources;

	for (i = 0; i < config->num_sources; ++i) {
		proc = &proc_cfg[i + proc_cfg_base_src];
		proc->cfg = &config->sources[i];
	}
	for (i = 0; i < config->num_destinations; ++i) {
		proc = &proc_cfg[i + proc_cfg_base_dst];
		proc->cfg = &config->destinations[i];
	}
} /* }}} */

static void destroy_msg_routes(const struct config_t * config) /* {{{ */
{
	size_t i;
	struct msg_route_t * route;

	if (!msg_routes) {
		return;
	}

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		if (route->filter && route->filter->free_ctx) {
			route->filter->free_ctx(&route->filter_ctx);
		}
	}
	free(msg_routes);
	msg_routes = NULL;
} /* }}} */

static void prepare_msg_routes(const struct config_t * config) /* {{{ */
{
	size_t i;
	struct msg_route_t * route;

	destroy_msg_routes(config);
	msg_routes = malloc(sizeof(struct msg_route_t) * config->num_routes);
	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		route->source = NULL;
		route->destination = NULL;
		route->filter = NULL;
		route->filter_cfg = NULL;
	}
} /* }}} */

static void handle_signal(int sig) /* {{{ */
{
	syslog(LOG_NOTICE, "pid:%d sig:%d\n", getpid(), sig);
	proc_set_request_terminate(1);
} /* }}} */

static void daemonize(void) /* {{{ */
{
	int rc;

	rc = fork();
	if (rc < 0) {
		syslog(LOG_CRIT, "unable to fork");
		exit(EXIT_FAILURE);
	}
	if (rc > 0) {
		exit(rc);
	}
} /* }}} */

static int proc_close(struct proc_config_t * proc) /* {{{ */
{
	if (proc->rfd >= 0) {
		close(proc->rfd);
		proc->rfd = -1;
	}
	if (proc->wfd >= 0) {
		close(proc->wfd);
		proc->wfd = -1;
	}
	return 0;
} /* }}} */

static int proc_close_wait(struct proc_config_t * proc) /* {{{ */
{
	proc_close(proc);
	waitpid(proc->pid, NULL, 0);
	proc->pid = -1;
	return 0;
} /* }}} */

static int proc_start(struct proc_config_t * proc, const struct proc_desc_t const * desc) /* {{{ */
{
	int rc_func;
	int rc;
	int rfd[2]; /* hub -> proc */
	int wfd[2]; /* proc -> hub */

	if (proc == NULL) return -1;
	if (proc->cfg == NULL) return -1;
	if (desc == NULL) return -1;
	if (desc->func == NULL) return -1;

	rc = pipe(rfd);
	if (rc < 0) {
		syslog(LOG_CRIT, "unable to create pipe for reading");
		return -1;
	}
	rc = pipe(wfd);
	if (rc < 0) {
		close(rfd[0]);
		close(rfd[1]);
		syslog(LOG_CRIT, "unable to create pipe for writing");
		return -1;
	}

	rc = fork();
	if (rc < 0) {
		close(rfd[0]);
		close(rfd[1]);
		close(wfd[0]);
		close(wfd[1]);
		syslog(LOG_CRIT, "cannot start proc '%s' (type: '%s')", proc->cfg->name, proc->cfg->type);
		return -1;
	}

	if (rc == 0) {
		/* child code */
		syslog(LOG_INFO, "start proc '%s' (type: '%s')", proc->cfg->name, proc->cfg->type);
		proc->pid = getpid();
		proc->rfd = rfd[0];
		proc->wfd = wfd[1];
		close(rfd[1]);
		close(wfd[0]);

		/* configure procedure */
		if (desc->configure) {
			rc = desc->configure(proc, &proc->cfg->properties);
			if (rc != EXIT_SUCCESS) {
				syslog(LOG_ERR, "invalid properties for proc type: '%s', stop proc '%s', rc=%d",
					proc->cfg->type, proc->cfg->name, rc);
				exit(rc);
			}
		}

		/* execute actual procedure */
		rc_func = desc->func(proc);
		syslog(LOG_INFO, "stop proc '%s', rc=%d", proc->cfg->name, rc_func);
		if (desc->clean) {
			rc = desc->clean(proc);
			if (rc != EXIT_SUCCESS) {
				syslog(LOG_ERR, "proc clean '%s', rc=%d", proc->cfg->name, rc);
			}
		}
		exit(rc_func);
	}

	/* parent code */
	proc->pid = rc;
	proc->rfd = wfd[0];
	proc->wfd = rfd[1];
	close(rfd[0]);
	close(wfd[1]);
	return rc;
} /* }}} */

/**
 * Dumps the registered objects (sources, destinations, filters) to the
 * specified stream.
 *
 * @param[in] out The stream to write the lists to.
 */
static void dump_registered(FILE * out)
{
	size_t i;

	fprintf(out, "Sources:\n");
	for (i = 0; i < desc_sources.num; ++i) {
		fprintf(out, " - %s\n", desc_sources.data[i].name);
	}
	fprintf(out, "Destinations:\n");
	for (i = 0; i < desc_destinations.num; ++i) {
		fprintf(out, " - %s\n", desc_destinations.data[i].name);
	}
	fprintf(out, "Filters:\n");
	for (i = 0; i < desc_filters.num; ++i) {
		fprintf(out, " - %s\n", desc_filters.data[i].name);
	}
}

static void config_dump_properties(FILE * file, const struct property_list_t const * properties) /* {{{ */
{
	size_t i;

	fprintf(file, "{");
	for (i = 0; i < properties->num; ++i) {
		const struct property_t const * prop = &properties->data[i];
		if (prop->value) {
			fprintf(file, " %s : %s", prop->key, prop->value);
		} else {
			fprintf(file, " %s", prop->key);
		}
		if (i < properties->num - 1) {
			fprintf(file, ", ");
		}
	}
	fprintf(file, " }");
} /* }}} */

static void config_dump(FILE * file, const struct config_t const * config) /* {{{ */
{
	size_t i;

	if (config == NULL) return;

	fprintf(file, "SOURCES\n");
	for (i = 0; i < config->num_sources; ++i) {
		struct proc_t * p = &config->sources[i];
		fprintf(file, " %s : %s ", p->name, p->type);
		config_dump_properties(file, &p->properties);
		fprintf(file, "\n");
	}
	fprintf(file, "DESTINATIONS\n");
	for (i = 0; i < config->num_destinations; ++i) {
		struct proc_t * p = &config->destinations[i];
		fprintf(file, " %s : %s ", p->name, p->type);
		config_dump_properties(file, &p->properties);
		fprintf(file, "\n");
	}
	fprintf(file, "FILTERS\n");
	for (i = 0; i < config->num_filters; ++i) {
		struct filter_t * p = &config->filters[i];
		fprintf(file, " %s : %s ", p->name, p->type);
		config_dump_properties(file, &p->properties);
		fprintf(file, "\n");
	}
	fprintf(file, "ROUTES\n");
	for (i = 0; i < config->num_routes; ++i) {
		struct route_t * p = &config->routes[i];
		fprintf(file, " %s --[%s]--> %s\n", p->name_source, p->name_filter, p->name_destination);
	}
} /* }}} */

/**
 * Reads the configuration and stores the valid information in the specified
 * structure.
 *
 * @param[out] config Structure containing the configuration of the file.
 * @retval  0 Success
 * @retval -1 Error in reading the configuration.
 * @retval -2 Error caused by invalid parameters.
 */
static int config_read(struct config_t * config)
{
	int rc;

	if (!config) {
		return -2;
	}

	if (option.config != 1) {
		syslog(LOG_CRIT, "configuration file name not defined.");
		return -1;
	}
	if (strlen(option.config_filename) <= 0) {
		syslog(LOG_CRIT, "invalid config file name: '%s'", option.config_filename);
		return -1;
	}
	config_init(config);
	rc = config_parse_file(option.config_filename, config);
	if (rc < 0) {
		syslog(LOG_CRIT, "unable to read config file '%s', rc=%d", option.config_filename, rc);
		return -1;
	}

	return 0;
}

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
		syslog(LOG_CRIT, "unable to send termination message");
		return -1;
	}
	return 0;
} /* }}} */

/**
 * Sets up the procedures (sources and destinations) and starts them.
 *
 * @param[in] num Number of procedures to process.
 * @param[in] base Starting index within the proc_cfg array, holding
 *   information about procedures.
 * @param[in] list List of procedure descriptors.
 * @retval  0 Success
 * @retval -1 Failure
 */
static int setup_procs(
		size_t num,
		size_t base,
		const struct proc_desc_list_t const * list)
{
	size_t i;
	int rc;
	const struct proc_desc_t const * desc = NULL;

	for (i = 0; i < num; ++i) {
		struct proc_config_t * ptr = &proc_cfg[i + base];
		desc = pdlist_find(list, ptr->cfg->type);
		if (desc == NULL) {
			syslog(LOG_ERR, "unknown proc type: '%s'", ptr->cfg->type);
			return -1;
		}
		rc = proc_start(ptr, desc);
		if (rc < 0) {
			return -1;
		}
	}
	return 0;
}

/**
 * Sets up the routes, consisting of a source and a destination with an optional
 * filter. The data structure used by the router is set up.
 *
 * @param[in] config The configuration data.
 * @retval  0 Success
 * @retval -1 Failure
 */
static int setup_routes(const struct config_t * config)
{
	size_t i;
	size_t j;
	struct msg_route_t * route;

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		route->source = NULL;
		route->destination = NULL;
		route->filter = NULL;
		route->filter_cfg = NULL;

		/* link source */
		for (j = 0; j < config->num_sources; ++j) {
			if (proc_cfg[j + proc_cfg_base_src].cfg == config->routes[i].source) {
				route->source = &proc_cfg[j + proc_cfg_base_src];
				break;
			}
		}

		/* link destination */
		for (j = 0; j < config->num_destinations; ++j) {
			if (proc_cfg[j + proc_cfg_base_dst].cfg == config->routes[i].destination) {
				route->destination = &proc_cfg[j + proc_cfg_base_dst];
				break;
			}
		}

		/* link initialized filter */
		if (config->routes[i].filter) {
			route->filter = filterlist_find(&desc_filters, config->routes[i].filter->type);
			if (route->filter) {
				route->filter_cfg = &config->routes[i].filter->properties;
				if (route->filter->configure) {
					if (route->filter->configure(&route->filter_ctx, route->filter_cfg)) {
						syslog(LOG_ERR, "%s:filter configuration failure: '%s'",
							__FUNCTION__, config->routes[i].name_filter);
						return -1;
					}
				}
			} else {
				syslog(LOG_ERR, "%s:unknown filter: '%s'", __FUNCTION__,
					config->routes[i].name_filter);
				return -1;
			}
		}
	}

	return 0;
}

/**
 * Routes a message sent by a source to a destination using an optional filter.
 * The routes are processed sequentially, using a filter in this context.
 *
 * @note Filters are running in the context of the main process, therefore
 *   it has to kept in mind to implement them in a manner as efficient as possible.
 *   Theoretically a filter does not consume any resources (especially time).
 *
 * @param[in] config The system configuration.
 * @param[in] source The source of the message.
 * @param[in] msg The message to send.
 * @retval  0 Success
 * @retval -1 Failure
 */
static int route_msg(
		const struct config_t * config,
		const struct proc_config_t * source,
		const struct message_t * msg)
{
	size_t i;
	int rc;
	struct msg_route_t * route;
	struct message_t out;

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		if (route->source != source) continue;

		/* execute filter if configured */
		if (route->filter) {
			memset(&out, 0, sizeof(out));
			rc = route->filter->func(&out, msg, &route->filter_ctx, route->filter_cfg);
			switch (rc) {
				case FILTER_SUCCESS:
					break;
				case FILTER_DISCARD:
					return 0;
				default:
				case FILTER_FAILURE:
					syslog(LOG_ERR, "filter error");
					return -1;
			}
		} else {
			memcpy(&out, msg, sizeof(out));
		}

		/* send message to destination */
		syslog(LOG_DEBUG, "route: %08x\n", msg->type);
		rc = write(route->destination->wfd, &out, sizeof(out));
		if (rc < 0) {
			syslog(LOG_CRIT, "unable to route message");
			return -1;
		}
	}
	return 0;
}

static void terminate_graceful(const struct config_t const * config) /* {{{ */
{
	size_t i;

	/* request subprocesses to terminate, gracefully, and wait for their termination */
	for (i = 0; i < config->num_sources + config->num_destinations; ++i) {
		send_terminate(&proc_cfg[i]);
	}
	for (i = 0; i < config->num_sources + config->num_destinations; ++i) {
		proc_close_wait(&proc_cfg[i]);
	}

	/* free resources */
	destroy_msg_routes(config);
	destroy_proc_configs();
	pdlist_free(&desc_sources);
	pdlist_free(&desc_destinations);
	filterlist_free(&desc_filters);
} /* }}} */

static void register_sources(void) /* {{{ */
{
	size_t i;

	pdlist_init(&desc_sources);

	pdlist_append(&desc_sources, &timer);

#ifdef ENABLE_SOURCE_GPSSERIAL
	pdlist_append(&desc_sources, &gps_serial);
#endif

#ifdef ENABLE_SOURCE_GPSSIMULATOR
	pdlist_append(&desc_sources, &gps_simulator);
#endif

#ifdef ENABLE_SOURCE_LUA
	pdlist_append(&desc_sources, &src_lua);
#endif

	for (i = 0; i < desc_sources.num; ++i) {
		config_register_source(desc_sources.data[i].name);
	}
} /* }}} */

static void register_destinations(void) /* {{{ */
{
	size_t i;

	pdlist_init(&desc_destinations);

	pdlist_append(&desc_destinations, &message_log);
	pdlist_append(&desc_destinations, &nmea_serial);
	pdlist_append(&desc_destinations, &logbook);

#ifdef ENABLE_DESTINATION_LUA
	pdlist_append(&desc_destinations, &dst_lua);
#endif

	for (i = 0; i < desc_destinations.num; ++i) {
		config_register_destination(desc_destinations.data[i].name);
	}
} /* }}} */

static void register_filters(void) /* {{{ */
{
	size_t i;

	filterlist_init(&desc_filters);

	filterlist_append(&desc_filters, &filter_null);
	filterlist_append(&desc_filters, &filter_nmea);

#ifdef ENABLE_FILTER_LUA
	filterlist_append(&desc_filters, &filter_lua);
#endif

	for (i = 0; i < desc_filters.num; ++i) {
		config_register_filter(desc_filters.data[i].name);
	}
} /* }}} */

int main(int argc, char ** argv) /* {{{ */
{
	size_t i;
	int graceful_termination = 0;

	struct message_t msg;
	int rc;
	fd_set rfds;
	int fd_max;
	int fd;

	sigset_t mask;
	struct sigaction act;

	struct config_t config;

	openlog(basename(argv[0]), LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);

	/* register known types (sources, destinations, filters) */
	register_sources();
	register_destinations();
	register_filters();

	/* command line arguments handling */
	if (parse_options(argc, argv) < 0) return EXIT_FAILURE;
	rc = setlogmask(LOG_UPTO(option.log_mask));
	if (rc < 0) {
		syslog(LOG_CRIT, "unable to set log mask");
		return EXIT_FAILURE;
	}

	/* list registered objects */
	if (option.list) {
		dump_registered(stdout);
		return EXIT_SUCCESS;
	}

	/* read configuration */
	if (config_read(&config) < 0) {
		return EXIT_FAILURE;
	}

	/* dump information */
	if (option.dump_config) {
		config_dump(stdout, &config);
		config_free(&config);
		return EXIT_SUCCESS;
	}

	/* daemonize process */
	if (option.daemonize) {
		daemonize();
	}

	/* set up signal handling (active also for all subprocesses, if masks are global) */
	proc_set_request_terminate(0);
	memset(&act, 0, sizeof(act));
	act.sa_handler = handle_signal;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		syslog(LOG_CRIT, "unable to signal action for SIGTERM");
		return EXIT_FAILURE;
	}
	if (sigaction(SIGINT, &act, NULL) < 0) {
		syslog(LOG_CRIT, "unable to signal action for SIGINT");
		return EXIT_FAILURE;
	}
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	if (sigprocmask(SIG_BLOCK, &mask, proc_get_signal_mask()) < 0) {
		syslog(LOG_CRIT, "unable to signal action for SIGTERM");
		return EXIT_FAILURE;
	}

	/* setup subprocesses */
	prepare_proc_configs(&config);
	if (setup_procs(config.num_destinations, proc_cfg_base_dst, &desc_destinations) < 0) {
		terminate_graceful(&config);
		return EXIT_FAILURE;
	}
	if (setup_procs(config.num_sources, proc_cfg_base_src, &desc_sources) < 0) {
		terminate_graceful(&config);
		return EXIT_FAILURE;
	}
	prepare_msg_routes(&config);
	if (setup_routes(&config) < 0) {
		terminate_graceful(&config);
		return EXIT_FAILURE;
	}

	/* main / hub process */
	for (; !proc_request_terminate() && !graceful_termination;) {
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

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, NULL, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_CRIT, "error in pselect: %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (proc_request_terminate()) {
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
				syslog(LOG_CRIT, "error in read: %s", strerror(errno));
				continue;
			}
			if (rc == 0) {
				syslog(LOG_WARNING, "process '%s' has given up.", proc_cfg[i].cfg->name);
				proc_close_wait(&proc_cfg[i]);
				graceful_termination = 1;
				break;
			}
			if (rc != (int)sizeof(msg)) {
				syslog(LOG_ERR, "cannot read message, rc=%d", rc);
				return EXIT_FAILURE;
			}

			if ((i >= proc_cfg_base_src) && (i < proc_cfg_base_dst)) {
				if (route_msg(&config, &proc_cfg[i], &msg) < 0) {
					syslog(LOG_DEBUG, "route: %08x", msg.type);
				}
			} else {
				syslog(LOG_DEBUG, "messages from destinations not supported yet.");
			}
		}

		/* terminate after max_msg */
		if (option.max_msg > 0) {
			--option.max_msg;
			if (option.max_msg == 0) {
				break;
			}
		}
	}

	terminate_graceful(&config);

	return EXIT_SUCCESS;
} /* }}} */

