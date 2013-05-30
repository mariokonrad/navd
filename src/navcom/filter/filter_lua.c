#include <navcom/filter/filter_lua.h>
#include <navcom/lua_helper.h>
#include <navcom/lua_syslog.h>
#include <navcom/lua_debug.h>
#include <navcom/lua_message.h>
#include <common/macros.h>
#include <syslog.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

struct filter_lua_data_t
{
	lua_State * lua;
	jmp_buf env;
};

static int panic(lua_State * lua)
{
	struct filter_lua_data_t * data;

	/* get jmpbuf from Lua state */
	lua_getglobal(lua, "__FILTER_DATA__");
	data = (struct filter_lua_data_t *)lua_touserdata(lua, -1);
	lua_pop(lua, 1);

	longjmp(data->env, -1);
	return 0;
}

/**
 * Returns a string to the Lua release.
 */
const char * filter_lua_release(void)
{
	return LUA_RELEASE;
}

static void init_data(struct filter_lua_data_t * data)
{
	memset(data, 0, sizeof(struct filter_lua_data_t));
}

static void setup_filter_results(lua_State * lua)
{
	luaH_define_const(lua, "FILTER_SUCCESS", FILTER_SUCCESS);
	luaH_define_const(lua, "FILTER_FAILURE", FILTER_FAILURE);
	luaH_define_const(lua, "FILTER_DISCARD", FILTER_DISCARD);
}

static int setup_lua_state(
		lua_State * lua,
		const struct property_t * debug_property)
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

	return EXIT_SUCCESS;
}

static int init_filter(
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	lua_State * lua = NULL;
	const struct property_t * prop_script = NULL;
	const struct property_t * prop_debug = NULL;
	struct filter_lua_data_t * data = NULL;

	if (ctx == NULL)
		return EXIT_FAILURE;
	if (properties == NULL)
		return EXIT_FAILURE;

	data = (struct filter_lua_data_t *)malloc(sizeof(struct filter_lua_data_t));
	ctx->data = data;
	init_data(data);

	prop_script = proplist_find(properties, "script");
	prop_debug = proplist_find(properties, "DEBUG");

	if (luaH_checkscript_from_prop(prop_script) != EXIT_SUCCESS)
		return EXIT_FAILURE;

	/* setup lua state */
	lua = luaL_newstate();
	if (lua == NULL) {
		syslog(LOG_ERR, "unable to create lua state");
		return EXIT_FAILURE;
	}

	/* introduce proc data to Lua state*/
	lua_pushlightuserdata(lua, data);
	lua_setglobal(lua, "__FILTER_DATA__");

	lua_atpanic(lua, panic);
	if (setjmp(data->env) == 0) {
		if (setup_lua_state(lua, prop_debug)) {
			syslog(LOG_ERR, "unable to setup lua state");
			return EXIT_FAILURE;
		}

		/* load/execute script */
		if (luaL_dofile(lua, prop_script->value) != LUA_OK) {
			syslog(LOG_ERR, "unable to load and execute script: '%s'", prop_script->value);
			lua_close(lua);
			return EXIT_FAILURE;
		}

		data->lua = lua;
		return EXIT_SUCCESS;
	} else {
		lua_atpanic(lua, NULL);
		syslog(LOG_CRIT, "LUA: %s", lua_tostring(lua, -1));
		lua_pop(lua, 1);
		lua_close(lua);
		return EXIT_FAILURE;
	}
}

int exit_filter(struct filter_context_t * ctx)
{
	struct filter_lua_data_t * data;

	if (ctx == NULL)
		return EXIT_FAILURE;
	if (ctx->data == NULL)
		return EXIT_FAILURE;

	data = (struct filter_lua_data_t *)ctx->data;
	if (data->lua) {
		lua_close(data->lua);
		data->lua = NULL;
	}

	free(data);
	ctx->data = NULL;
	return EXIT_SUCCESS;
}

/**
 * @todo Implement Lua error handling
 */
static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	int rc;
	struct filter_lua_data_t * data = NULL;

	UNUSED_ARG(properties);

	if (out == NULL)
		return FILTER_FAILURE;
	if (in == NULL)
		return FILTER_FAILURE;
	if (ctx == NULL)
		return FILTER_FAILURE;
	if (ctx->data == NULL)
		return FILTER_FAILURE;

	data = (struct filter_lua_data_t *)ctx->data;

	if (setjmp(data->env) == 0) {
		lua_getglobal(data->lua, "filter");
		lua_pushlightuserdata(data->lua, (void*)out);
		lua_pushlightuserdata(data->lua, (void*)in);
		rc = luaH_check_error(data->lua, lua_pcall(data->lua, 2, 1, 0));

		if (rc == EXIT_SUCCESS) {
			rc = luaL_checkinteger(data->lua, -1);
			lua_pop(data->lua, lua_gettop(data->lua));
			return rc;
		} else {
			lua_pop(data->lua, lua_gettop(data->lua));
			return FILTER_FAILURE;
		}
	} else {
		lua_atpanic(data->lua, NULL);
		syslog(LOG_CRIT, "LUA: %s", lua_tostring(data->lua, -1));
		lua_pop(data->lua, 1);
		lua_close(data->lua);
		data->lua = NULL;
		return FILTER_FAILURE;
	}
}

static void help(void)
{
	printf("\n");
	printf("filter_lua\n");
	printf("\n");
	printf("This filter is scriptable in Lua.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  script : filename of the Lua script to execute.\n");
	printf("  DEBUG  : [optional] a combination of 'c', 'r' and 'l' for debugging purposes.\n");
	printf("           c : call, traces function calls\n");
	printf("           r : return, traces function returns\n");
	printf("           l : line, traces executed lines\n");
	printf("\n");
	printf("Example:\n");
	printf("\n");
	printf("  timer_1s : timer { id:1, period:1000 };\n");
	printf("  filter1 : filter_lua { script:'src/test/testfilter.lua' };\n");
	printf("  log : message_log {};\n");
	printf("  timer_1s -> [filter1] -> log;\n");
	printf("\n");
	printf("Example of Lua script:\n");
	printf("\n");
	printf("  function filter(msg_out, msg_in)\n");
	printf("    msg_clone(msg_out, msg_in)\n");
	printf("    return FILTER_SUCCESS\n");
	printf("  end\n");
	printf("\n");
}

const struct filter_desc_t filter_lua = {
	.name = "filter_lua",
	.init = init_filter,
	.exit = exit_filter,
	.func = filter,
	.help = help,
};

