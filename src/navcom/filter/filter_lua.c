#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <common/fileutil.h>
#include <string.h>
#include <syslog.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

static int lua__syslog(lua_State * lua)
{
	int type = -1;
	const char * str = NULL;

	type = luaL_checkinteger(lua, -2);
	str = luaL_checkstring(lua, -1);

	syslog(type, "%s", str);
	return 0;
}

static void define_const(lua_State * lua, const char * sym, int val)
{
	lua_pushinteger(lua, val);
	lua_setglobal(lua, sym);
}

static int setup_lua_state(lua_State * lua)
{
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);

	/* filter result values */
	define_const(lua, "FILTER_SUCCESS", FILTER_SUCCESS);
	define_const(lua, "FILTER_FAILURE", FILTER_FAILURE);
	define_const(lua, "FILTER_DISCARD", FILTER_DISCARD);

	/* setup syslog */
	define_const(lua, "LOG_CRIT",    LOG_CRIT);
	define_const(lua, "LOG_ERR",     LOG_ERR);
	define_const(lua, "LOG_WARNING", LOG_WARNING);
	define_const(lua, "LOG_DEBUG",   LOG_DEBUG);
	define_const(lua, "LOG_NOTICE",  LOG_NOTICE);
	lua_register(lua, "syslog", lua__syslog);

	/* TODO: setup message handling functions */

	/* TODO: setup error handling */

	/* TODO: setup debug interface if property is set */

	return 0;
}

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

	/* setup lua state */
	lua = luaL_newstate();
	if (!lua) {
		syslog(LOG_ERR, "unable to create lua state");
		return FILTER_FAILURE;
	}
	if (setup_lua_state(lua)) {
		syslog(LOG_ERR, "unable to setup lua state");
		return FILTER_FAILURE;
	}

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
	int rc;
	lua_State * lua = NULL;

	UNUSED_ARG(out);
	UNUSED_ARG(in);
	UNUSED_ARG(properties);

	if (!ctx || !ctx->data) return FILTER_FAILURE;
	lua = (lua_State *)ctx->data;

	lua_getglobal(lua, "filter");
	lua_pushlightuserdata(lua, (void*)out);
	lua_pushlightuserdata(lua, (void*)in);
	rc = lua_pcall(lua, 2, 1, 0);
	if (rc != LUA_OK) {
		const char * err_string = lua_tostring(lua, -1);
		switch (rc) {
			case LUA_ERRRUN:
				syslog(LOG_ERR, "runtime error: '%s'", err_string);
				break;
			case LUA_ERRMEM:
				syslog(LOG_ERR, "memory allocation error: '%s'", err_string);
				break;
			case LUA_ERRERR:
				syslog(LOG_ERR, "message handler error: '%s'", err_string);
				break;
			case LUA_ERRGCMM:
				syslog(LOG_ERR, "error while calling metamethod: '%s'", err_string);
				break;
		}
		lua_pop(lua, 1);
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

