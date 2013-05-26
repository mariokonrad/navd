#include <route.h>
#include <registry.h>
#include <config/config.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <navcom/proc.h>
#include <navcom/filter.h>
#include <navcom/filter_list.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

/**
 * Structure to hold all runtime information about a route
 * for messages from sources through filters to destinations.
 */
struct msg_route_t {
	/**
	 * Source of a message. This information is mandatory.
	 */
	const struct proc_config_t * source;

	/**
	 * Destination of a message. This information is mandatory.
	 */
	const struct proc_config_t * destination;

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
 * Array of runtime information of all configured routes.
 */
static struct msg_route_t * msg_routes = NULL;

/**
 * Frees all resources held by all routes.
 */
void route_destroy(const struct config_t * config)
{
	size_t i;
	struct msg_route_t * route;

	if (msg_routes == NULL)
		return;

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		if (route->filter && route->filter->exit) {
			route->filter->exit(&route->filter_ctx);
		}
	}
	free(msg_routes);
	msg_routes = NULL;
}

/**
 * Initializes all data structures necessary to handle routes.
 */
void route_init(const struct config_t * config)
{
	size_t i;
	struct msg_route_t * route;

	route_destroy(config);
	msg_routes = malloc(sizeof(struct msg_route_t) * config->num_routes);
	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		route->source = NULL;
		route->destination = NULL;
		route->filter = NULL;
		route->filter_cfg = NULL;
	}
}

static void link_route_sources(
		struct msg_route_t * route,
		const struct config_t * config,
		size_t route_config_index,
		const struct proc_config_t * proc_conf,
		size_t proc_conf_base)
{
	size_t j;

	for (j = 0; j < config->num_sources; ++j) {
		if (proc_conf[j + proc_conf_base].cfg == config->routes[route_config_index].source) {
			route->source = &proc_conf[j + proc_conf_base];
			break;
		}
	}
}

static void link_route_destinations(
		struct msg_route_t * route,
		const struct config_t * config,
		size_t route_config_index,
		const struct proc_config_t * proc_conf,
		size_t proc_conf_base)
{
	size_t j;

	for (j = 0; j < config->num_destinations; ++j) {
		if (proc_conf[j + proc_conf_base].cfg == config->routes[route_config_index].destination) {
			route->destination = &proc_conf[j + proc_conf_base];
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
int route_setup(
		const struct config_t * config,
		const struct proc_config_t * proc_conf,
		size_t proc_conf_base_src,
		size_t proc_conf_base_dst)
{
	size_t i;
	struct msg_route_t * route;

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		route->source = NULL;
		route->destination = NULL;
		route->filter = NULL;
		route->filter_cfg = NULL;

		link_route_sources(route, config, i, proc_conf, proc_conf_base_src);
		link_route_destinations(route, config, i, proc_conf, proc_conf_base_dst);

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
int route_msg(
		const struct config_t * config,
		const struct proc_config_t * source,
		const struct message_t * msg)
{
	size_t i;
	int rc;
	struct msg_route_t * route;
	struct message_t out;

	if (config == NULL)
		return -1;
	if (source == NULL)
		return -1;
	if (msg == NULL)
		return -1;

	for (i = 0; i < config->num_routes; ++i) {
		route = &msg_routes[i];
		if (route->source != source)
			continue;

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
		if (message_write(route->destination->wfd, &out) != EXIT_SUCCESS) {
			syslog(LOG_CRIT, "unable to route message");
			return -1;
		}
	}
	return 0;
}

