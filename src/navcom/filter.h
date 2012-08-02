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
 * Prototype of a filter function to process messages.
 */
typedef int (*filter_function)(
		struct message_t *,
		const struct message_t *,
		const struct property_list_t *);

/**
 * Structure representing the description of a filter, holding all
 * necessary information.
 *
 * @note It is not advised for filters to have static data.
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
	 * The filters function to be called every time the filter
	 * has to process a message.
	 */
	filter_function func;
};

#endif
