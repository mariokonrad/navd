#include <navcom/source/src_lua.h>
#include <navcom/lua_helper.h>
#include <navcom/lua_syslog.h>
#include <navcom/lua_debug.h>
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

/* TODO: move static data into config->data */
static int initialized = 0;
static struct timespec tm_cfg;
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
const char * src_lua_release(void)
{
	return LUA_RELEASE;
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

	/* TODO: setup message handling */

	return EXIT_SUCCESS;
}

/**
 * Calls the Lua state to handle the next period.
 *
 * @reval EXIT_SUCCESS
 * @reval EXIT_FAILURE
 */
static int handle_script(const struct proc_config_t * config)
{
	struct message_t msg;
	lua_State * lua = (lua_State *)config->data;
	int rc;

	memset(&msg, 0, sizeof(msg));

	if (setjmp(env) == 0) {
		lua_getglobal(lua, "handle");
		lua_pushlightuserdata(lua, (void*)&msg);
		rc = lua_pcall(lua, 1, 0, 0);
		if (rc == LUA_OK) {
			rc = luaL_checkinteger(lua, -1);
			lua_pop(lua, 1);
			if (rc) {
				/* TODO: check message integrity */
				if (message_write(config->wfd, &msg) != EXIT_SUCCESS)
					return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		} else {
			luaH_check_error(lua, rc);
		}
	} else {
		lua_atpanic(lua, NULL);
		syslog(LOG_CRIT, "LUA: %s", lua_tostring(lua, -1));
		lua_pop(lua, 1);
	}
	return EXIT_FAILURE;
}

static int setup_period(const struct property_t * property)
{
	char * endptr = NULL;
	uint32_t t;

	if (!property) {
		syslog(LOG_ERR, "no period defined");
		return EXIT_FAILURE;
	}
	t = strtoul(property->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value in period: '%s'", property->value);
		return EXIT_FAILURE;
	}
	tm_cfg.tv_sec = t / 1000;
	tm_cfg.tv_nsec = (t % 1000);
	tm_cfg.tv_nsec *= 1000000;

	initialized = 1;
	return EXIT_SUCCESS;
}

static int init_proc(
		struct proc_config_t * config,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;
	const struct property_t * prop_script = NULL;
	const struct property_t * prop_period = NULL;
	const struct property_t * prop_debug = NULL;

	prop_script = proplist_find(properties, "script");
	prop_period = proplist_find(properties, "period");
	prop_debug = proplist_find(properties, "DEBUG");

	rc = setup_period(prop_period);
	if (rc != EXIT_SUCCESS)
		return rc;

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
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	if (!initialized) {
		syslog(LOG_ERR, "uninitialized");
		return EXIT_FAILURE;
	}

	while (!proc_request_terminate()) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max)
			fd_max = config->rfd;

		tm = tm_cfg;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, proc_get_signal_mask());
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timerout */
			if (handle_script(config) != EXIT_SUCCESS)
				return EXIT_FAILURE;
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
				default:
					break;
			}
			continue;
		}
	}

	return EXIT_SUCCESS;
}

const struct proc_desc_t src_lua = {
	.name = "src_lua",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
};

