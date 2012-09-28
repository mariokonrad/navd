#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <string.h>
#include <syslog.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

static int configure(
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	lua_State * lua = NULL;

	UNUSED_ARG(properties);

	/* TODO: property: script */

	lua = luaL_newstate();
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);
	ctx->data = lua;

	return FILTER_SUCCESS;
}

int free_context(
		struct filter_context_t * ctx)
{
	if (ctx && ctx->data) {
		lua_State * lua = (lua_State *)ctx->data;
		lua_close(lua);
		ctx->data = NULL;
	}

	return FILTER_SUCCESS;
}

static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	lua_State * lua = NULL;

	UNUSED_ARG(out);
	UNUSED_ARG(in);
	UNUSED_ARG(properties);

	if (!ctx || !ctx->data) return FILTER_FAILURE;
	lua = (lua_State *)ctx->data;

	/* TODO */

	return FILTER_DISCARD;
}

const struct filter_desc_t filter_lua = {
	.name = "filter_lua",
	.configure = configure,
	.free_ctx = free_context,
	.func = filter
};

