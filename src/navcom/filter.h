#ifndef __NAVCOM__FILTER__H__
#define __NAVCOM__FILTER__H__

#include <navcom/message.h>
#include <common/property.h>

/**
 * Positive result of a filter operation.
 */
#define FILTER_SUCCESS 0

/**
 * Result of a filter operation to indicate a failure to
 * process a message.
 */
#define FILTER_FAILURE -1

/**
 * Filter telling the caller to discard the message. The
 * fiter indicates to discard the message without being
 * processed by the filter, this means it does not have
 * to be sent to the destination.
 */
#define FILTER_DISCARD 1

/**
 * Structure to hold filter instance specific data.
 */
struct filter_context_t {
	void * data;
};

/**
 * Prototype of a filter function to process messages.
 *
 * @retval FILTER_SUCCESS
 * @retval FILTER_FAILURE
 * @retval FILTER_DISCARD
 */
typedef int (*filter_function)(
		struct message_t *,
		const struct message_t *,
		struct filter_context_t *,
		const struct property_list_t *);

/**
 * Prototype of a filter configuration function.
 *
 * @retval  0 Success
 * @retval -1 Failure
 */
typedef int (*filter_config_function)(
		struct filter_context_t *,
		const struct property_list_t *);

/**
 * Prototype for a function to free the filters context data.
 *
 * @retval  0 Success
 * @retval -1 Failure
 */
typedef int (*filter_free_context_function)(
		struct filter_context_t *);

/**
 * Structure representing the description of a filter, holding all
 * necessary information.
 *
 * @note It is not advised for filters to have static data. Use the
 *  filter context instead, it can be unique for every route, but
 *  different for the same filter functions.
 *
 * @todo A setup function is needed similar to the procedures, to avoid
 *  the parsing of the properties every time the filter is executed.
 */
struct filter_desc_t {
	/**
	 * The filters name. This name is used to identify the filter
	 * within the configuration file.
	 *
	 * @note Do not use whitespaces.
	 */
	const char * name;

	/**
	 * The filters configuration function.
	 */
	filter_config_function configure;

	/**
	 * Frees the specific data, possibly allocated by the
	 * configure function.
	 */
	filter_free_context_function free_ctx;

	/**
	 * The filters function to be called every time the filter
	 * has to process a message.
	 */
	filter_function func;
};

#endif
