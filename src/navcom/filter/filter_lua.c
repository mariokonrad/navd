#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <common/fileutil.h>
#include <string.h>
#include <syslog.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

static int configure(
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;
	const struct property_t * prop = NULL;

	prop = proplist_find(properties, "script");
	if (!prop) {
		syslog(LOG_ERR, "no script defined");
		return FILTER_FAILURE;
	}

	if (!file_is_readable(prop->value)) {
		syslog(LOG_ERR, "file not readable: '%s'", prop->value);
		return FILTER_FAILURE;
	}

	/* initialize lua state */
	lua = luaL_newstate();
	if (!lua) {
		syslog(LOG_ERR, "unable to create lua state");
		return FILTER_FAILURE;
	}
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);
	lua_pushinteger(lua, FILTER_SUCCESS);
	lua_setglobal(lua, "FILTER_SUCCESS");
	lua_pushinteger(lua, FILTER_FAILURE);
	lua_setglobal(lua, "FILTER_FAILURE");
	lua_pushinteger(lua, FILTER_DISCARD);
	lua_setglobal(lua, "FILTER_DISCARD");

	/* TODO: setup error handling */
	/* TODO: setup debug interface if property is set */

	/* load/execute script */
	rc = luaL_dofile(lua, prop->value);
	if (rc) {
		syslog(LOG_ERR, "unable to load and execute script: '%s'", prop->value);
		lua_close(lua);
		return FILTER_FAILURE;
	}

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

	lua_pushlightuserdata(lua, (void*)out);
	lua_pushlightuserdata(lua, (void*)in);

	lua_getglobal(lua, "filter");
	if (lua_pcall(lua, 2, 1, 0)) {
		/* error ocurred */
		/* TODO: write error to syslog */
		return FILTER_FAILURE;
	}

	return luaL_checkinteger(lua, -1);
}

const struct filter_desc_t filter_lua = {
	.name = "filter_lua",
	.configure = configure,
	.free_ctx = free_context,
	.func = filter
};

