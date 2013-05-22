#include <navcom/destination/dst_lua.h>
#include <navcom/lua_helper.h>
#include <navcom/lua_syslog.h>
#include <navcom/lua_debug.h>
#include <navcom/lua_message.h>
#include <navcom/message.h>
#include <navcom/message_comm.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <setjmp.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

static jmp_buf env;

static int panic(lua_State * lua)
{
	UNUSED_ARG(lua);

	longjmp(env, -1);
	return 0;
}

/**
 * Returns a string to the Lua release.
 */
const char * dst_lua_release(void)
{
	return LUA_RELEASE;
}

/**
 * Processes the specified message.
 *
 * @reval EXIT_SUCCESS
 * @reval EXIT_FAILURE
 */
static int process_message(lua_State * lua, const struct message_t * msg)
{
	int rc;

	if (!lua)
		return EXIT_FAILURE;
	if (!msg)
		return EXIT_FAILURE;

	if (setjmp(env) == 0) {
		lua_getglobal(lua, "handle");
		lua_pushlightuserdata(lua, (void*)msg);
		rc = lua_pcall(lua, 1, 0, 0);
		luaH_check_error(lua, rc);
		return EXIT_SUCCESS;
	} else {
		lua_atpanic(lua, NULL);
		syslog(LOG_CRIT, "LUA: %s", lua_tostring(lua, -1));
		lua_pop(lua, 1);
		return EXIT_FAILURE;
	}
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

	return EXIT_SUCCESS;
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;
	const struct property_t * prop_script = NULL;
	const struct property_t * prop_debug = NULL;

	prop_script = proplist_find(properties, "script");
	prop_debug = proplist_find(properties, "DEBUG");

	rc = luaH_checkscript_from_prop(prop_script);
	if (rc != EXIT_SUCCESS)
		return rc;

	/* setup lua state */
	lua = luaL_newstate();
	if (!lua) {
		syslog(LOG_ERR, "unable to create lua state");
		return EXIT_FAILURE;
	}
	lua_atpanic(lua, panic);
	if (setjmp(env) == 0) {
		if (setup_lua_state(lua, prop_debug)) {
			syslog(LOG_ERR, "unable to setup lua state");
			return EXIT_FAILURE;
		}

		/* load/execute script */
		rc = luaL_dofile(lua, prop_script->value);
		if (rc) {
			syslog(LOG_ERR, "unable to load and execute script: '%s'", prop_script->value);
			lua_close(lua);
			return EXIT_FAILURE;
		}

		config->data = lua;
		return EXIT_SUCCESS;
	} else {
		lua_atpanic(lua, NULL);
		syslog(LOG_CRIT, "LUA: %s", lua_tostring(lua, -1));
		lua_pop(lua, 1);
		lua_close(lua);
		return EXIT_FAILURE;
	}
}

/**
 * Cleans up.
 *
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int exit_proc(struct proc_config_t * config)
{
	if (!config)
		return EXIT_FAILURE;

	if (config->data) {
		lua_close((lua_State *)config->data);
		config->data = NULL;
	}
	return EXIT_SUCCESS;
}

static int proc(struct proc_config_t * config)
{
	int rc;
	fd_set rfds;
	struct message_t msg;

	while (!proc_request_terminate()) {
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);

		rc = pselect(config->rfd + 1, &rfds, NULL, NULL, NULL, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->rfd, &rfds)) {
			if (message_read(config->rfd, &msg) != EXIT_SUCCESS)
				return EXIT_FAILURE;
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							return EXIT_SUCCESS;
						default:
							break;
					}
					break;

				case MSG_TIMER:
				case MSG_NMEA:
					if (process_message((lua_State *)config->data, &msg) != EXIT_SUCCESS) {
						return EXIT_FAILURE;
					}
					break;

				default:
					syslog(LOG_WARNING, "unknown msg type: %08x\n", msg.type);
					break;
			}
			continue;
		}
	}

	return EXIT_SUCCESS;
}

const struct proc_desc_t dst_lua = {
	.name = "dst_lua",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
};

