#include <navcom/filter/filter_null.h>
#include <common/macros.h>
#include <string.h>

/**
 * The null filter copies the original message to the result
 * without manipulating anything or filtering out messages.
 *
 * @param[out] out The resulting message, an exact copy of the
 *  original.
 * @param[in] in The original message which will be copied.
 * @param[inout] ctx The filters context.
 * @param[in] properties Properties of the filter. As of now
 *  they are parsed every time the filter is executed.
 */
static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	UNUSED_ARG(ctx);
	UNUSED_ARG(properties);

	if (out == NULL) {
		return FILTER_FAILURE;
	}
	if (in == NULL) {
		return FILTER_FAILURE;
	}

	memcpy(out, in, sizeof(struct message_t));
	return FILTER_SUCCESS;
}

const struct filter_desc_t filter_null = {
	.name = "filter_null",
	.configure = NULL,
	.free_ctx = NULL,
	.func = filter
};

