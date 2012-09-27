#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <string.h>
#include <syslog.h>
#include <lua/lua.h>

static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	UNUSED_ARG(out);
	UNUSED_ARG(in);
	UNUSED_ARG(ctx);
	UNUSED_ARG(properties);

	/* TODO */

	return FILTER_FAILURE;
}

const struct filter_desc_t filter_nmea = {
	.name = "filter_lua",
	.configure = NULL,
	.free_ctx = NULL,
	.func = filter
};

