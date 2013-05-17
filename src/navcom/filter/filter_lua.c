#include <navcom/filter/filter_lua.h>
#include <navcom/lua_helper.h>
#include <navcom/lua_syslog.h>
#include <navcom/lua_debug.h>
#include <navcom/lua_message.h>
#include <common/macros.h>
#include <syslog.h>
#include <stdlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

/**
 * Returns a string to the Lua release.
 */
const char * filter_lua_release(void)
{
	return LUA_RELEASE;
}

static void setup_filter_results(lua_State * lua)
{
	luaH_define_const(lua, "FILTER_SUCCESS", FILTER_SUCCESS);
	luaH_define_const(lua, "FILTER_FAILURE", FILTER_FAILURE);
	luaH_define_const(lua, "FILTER_DISCARD", FILTER_DISCARD);
}

static int setup_lua_state(lua_State * lua, const struct property_t * debug_property)
{
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);

	luaH_define_const(lua, "EXIT_SUCCESS", EXIT_SUCCESS);
	luaH_define_const(lua, "EXIT_FAILURE", EXIT_FAILURE);

	luaH_setup_syslog(lua);
	luaH_setup_debug(lua, debug_property);
	luaH_setup_message_handling(lua);
	setup_filter_results(lua);

	/* TODO: setup error handling */

	return EXIT_SUCCESS;
}

static int configure(
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;
	const struct property_t * prop_script = NULL;
	const struct property_t * prop_debug = NULL;

	prop_script = proplist_find(properties, "script");
	prop_debug = proplist_find(properties, "DEBUG");

	rc = luaH_checkscript_from_prop(prop_script);
	if (rc != EXIT_SUCCESS) return rc;

	/* setup lua state */
	lua = luaL_newstate();
	if (!lua) {
		syslog(LOG_ERR, "unable to create lua state");
		return FILTER_FAILURE;
	}
	if (setup_lua_state(lua, prop_debug)) {
		syslog(LOG_ERR, "unable to setup lua state");
		return FILTER_FAILURE;
	}

	/* load/execute script */
	rc = luaL_dofile(lua, prop_script->value);
	if (rc) {
		syslog(LOG_ERR, "unable to load and execute script: '%s'", prop_script->value);
		lua_close(lua);
		return FILTER_FAILURE;
	}

	ctx->data = lua;
	return FILTER_SUCCESS;
}

int free_context(struct filter_context_t * ctx)
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

