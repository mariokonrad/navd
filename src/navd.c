#include <global_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <syslog.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <libgen.h>
#include <unistd.h>

#include <daemon.h>
#include <programoptions.h>
#include <registry.h>
#include <common/macros.h>
#include <config/config.h>
#include <navcom/message.h>
#include <navcom/proc_list.h>
#include <navcom/filter_list.h>

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

static void destroy_proc_configs(void)
{
	if (proc_cfg) {
		free(proc_cfg);
		proc_cfg = NULL;
	}
}

static void prepare_proc_configs(const struct config_t * config)
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
}

static void destroy_msg_routes(const struct config_t * config)
{
	size_t i;
	struct msg_route_t * route;

	if (!msg_routes) {
		return;
	}

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		if (route->filter && route->filter->exit) {
			route->filter->exit(&route->filter_ctx);
		}
	}
	free(msg_routes);
	msg_routes = NULL;
}

static void prepare_msg_routes(const struct config_t * config)
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
}

static void handle_signal(int sig)
{
	syslog(LOG_NOTICE, "pid:%d sig:%d\n", getpid(), sig);
	proc_set_request_terminate(1);
}

static int proc_close(struct proc_config_t * proc)
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
}

static int proc_close_wait(struct proc_config_t * proc)
{
	proc_close(proc);
	waitpid(proc->pid, NULL, 0);
	proc->pid = -1;
	return 0;
}

static int proc_start(struct proc_config_t * proc, const struct proc_desc_t const * desc)
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

		/* initialize procedure */
		if (desc->init) {
			rc = desc->init(proc, &proc->cfg->properties);
			if (rc != EXIT_SUCCESS) {
				syslog(LOG_ERR, "initialization failure for proc type: '%s', stop proc '%s', rc=%d",
					proc->cfg->type, proc->cfg->name, rc);

				/* try to clean up if init was not completely successful */
				if (desc->exit)
					if (desc->exit(proc) != EXIT_SUCCESS)
						syslog(LOG_ERR, "proc short exit '%s'", proc->cfg->name);

				exit(rc);
			}
		}

		/* execute actual procedure */
		rc_func = desc->func(proc);
		syslog(LOG_INFO, "stop proc '%s', rc=%d", proc->cfg->name, rc_func);

		/* clean up */
		if (desc->exit) {
			if (desc->exit(proc) != EXIT_SUCCESS)
				syslog(LOG_ERR, "proc exit '%s', rc=%d", proc->cfg->name, rc);
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
}

static void config_dump_properties(const struct property_list_t const * properties)
{
	size_t i;

	printf("{");
	for (i = 0; i < properties->num; ++i) {
		const struct property_t const * prop = &properties->data[i];
		if (prop->value) {
			printf(" %s : %s", prop->key, prop->value);
		} else {
			printf(" %s", prop->key);
		}
		if (i < properties->num - 1)
			printf(", ");
	}
	printf(" }");
}

static void config_dump(const struct config_t const * config)
{
	size_t i;

	if (config == NULL) return;

	printf("SOURCES\n");
	for (i = 0; i < config->num_sources; ++i) {
		struct proc_t * p = &config->sources[i];
		printf(" %s : %s ", p->name, p->type);
		config_dump_properties(&p->properties);
		printf("\n");
	}
	printf("DESTINATIONS\n");
	for (i = 0; i < config->num_destinations; ++i) {
		struct proc_t * p = &config->destinations[i];
		printf(" %s : %s ", p->name, p->type);
		config_dump_properties(&p->properties);
		printf("\n");
	}
	printf("FILTERS\n");
	for (i = 0; i < config->num_filters; ++i) {
		struct filter_t * p = &config->filters[i];
		printf(" %s : %s ", p->name, p->type);
		config_dump_properties(&p->properties);
		printf("\n");
	}
	printf("ROUTES\n");
	for (i = 0; i < config->num_routes; ++i) {
		struct route_t * p = &config->routes[i];
		printf(" %s --[%s]--> %s\n", p->name_source, p->name_filter, p->name_destination);
	}
}

/**
 * Reads the configuration and stores the valid information in the specified
 * structure.
 *
 * @param[out] config Structure containing the configuration of the file.
 * @param[in] options Program options.
 * @retval  0 Success
 * @retval -1 Error in reading the configuration.
 * @retval -2 Error caused by invalid parameters.
 */
static int config_read(
		struct config_t * config,
		const struct options_data_t * options)
{
	int rc;

	if (!config) {
		return -2;
	}

	if (options->config != 1) {
		syslog(LOG_CRIT, "configuration file name not defined.");
		return -1;
	}
	if (strlen(options->config_filename) <= 0) {
		syslog(LOG_CRIT, "invalid config file name: '%s'", options->config_filename);
		return -1;
	}
	config_init(config);
	rc = config_parse_file(options->config_filename, config);
	if (rc < 0) {
		syslog(LOG_CRIT, "unable to read config file '%s', rc=%d", options->config_filename, rc);
		return -1;
	}

	return 0;
}

static int send_terminate(const struct proc_config_t * proc)
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
}

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
		if (rc < 0)
			return -1;
	}
	return 0;
}

static void link_route_sources(
		struct msg_route_t * route,
		const struct config_t * config,
		size_t route_config_index)
{
	size_t j;

	for (j = 0; j < config->num_sources; ++j) {
		if (proc_cfg[j + proc_cfg_base_src].cfg == config->routes[route_config_index].source) {
			route->source = &proc_cfg[j + proc_cfg_base_src];
			break;
		}
	}
}

static void link_route_destinations(
		struct msg_route_t * route,
		const struct config_t * config,
		size_t route_config_index)
{
	size_t j;

	for (j = 0; j < config->num_destinations; ++j) {
		if (proc_cfg[j + proc_cfg_base_dst].cfg == config->routes[route_config_index].destination) {
			route->destination = &proc_cfg[j + proc_cfg_base_dst];
			break;
		}
	}
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
	struct msg_route_t * route;

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		route->source = NULL;
		route->destination = NULL;
		route->filter = NULL;
		route->filter_cfg = NULL;

		link_route_sources(route, config, i);
		link_route_destinations(route, config, i);

		/* link initialized filter */
		if (config->routes[i].filter) {
			route->filter = filterlist_find(registry_filters(), config->routes[i].filter->type);
			if (route->filter) {
				route->filter_cfg = &config->routes[i].filter->properties;
				if (route->filter->init) {
					if (route->filter->init(&route->filter_ctx, route->filter_cfg)) {
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

static void terminate_graceful(const struct config_t const * config)
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
	registry_free();
}

static int setup_signal_handling(void)
{
	sigset_t mask;
	struct sigaction act;

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
	return EXIT_SUCCESS;
}

int main(int argc, char ** argv)
{
	size_t i;
	int graceful_termination = 0;
	struct message_t msg;
	int rc;
	fd_set rfds;
	int fd_max;
	int fd;
	struct config_t config;
	struct options_data_t option;

	openlog(basename(argv[0]), LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
	registry_register();

	/* command line arguments handling */
	if (parse_options(argc, argv, &option) < 0)
		return EXIT_FAILURE;

	if (option.help) {
		if (strlen(option.help_specific) > 0) {
			registry_print_help_for(option.help_specific);
		} else {
			print_usage(argv[0]);
		}
		return EXIT_FAILURE;
	}

	if (setlogmask(LOG_UPTO(option.log_mask)) < 0) {
		syslog(LOG_CRIT, "unable to set log mask");
		return EXIT_FAILURE;
	}

	/* list registered objects */
	if (option.list || option.list_compact) {
		registry_dump(option.list_compact);
		return EXIT_SUCCESS;
	}

	/* read configuration */
	if (config_read(&config, &option) < 0)
		return EXIT_FAILURE;

	/* dump information */
	if (option.dump_config) {
		config_dump(&config);
		config_free(&config);
		return EXIT_SUCCESS;
	}

	/* daemonize process */
	if (option.daemonize)
		daemonize();

	if (setup_signal_handling() != EXIT_SUCCESS)
		return EXIT_FAILURE;

	/* setup subprocesses */
	prepare_proc_configs(&config);
	if (setup_procs(config.num_destinations, proc_cfg_base_dst, registry_destinations()) < 0) {
		terminate_graceful(&config);
		return EXIT_FAILURE;
	}
	if (setup_procs(config.num_sources, proc_cfg_base_src, registry_sources()) < 0) {
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
			if (fd <= 0)
				continue;
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
			if (fd <= 0)
				continue;
			if (!FD_ISSET(fd, &rfds))
				continue;

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
}

