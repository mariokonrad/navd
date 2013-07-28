#include <navcom/destination/dst_lua.h>
#include <navcom/destination/dst_lua_private.h>
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
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <sys/signalfd.h>

static int panic(lua_State * lua)
{
	struct dst_lua_data_t * data;

	/* get jmpbuf from Lua state */
	lua_getglobal(lua, "__PROC_DATA__");
	data = (struct dst_lua_data_t *)lua_touserdata(lua, -1);
	lua_pop(lua, 1);

	longjmp(data->env, -1);
	return 0;
}

static void init_data(struct dst_lua_data_t * data)
{
	memset(data, 0, sizeof(struct dst_lua_data_t));
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
 * @reval EXIT_SUCCESS Success.
 * @reval EXIT_FAILURE Failure.
 *
 * @todo Test
 */
static int process_message(
		struct dst_lua_data_t * data,
		const struct message_t * msg)
{
	if (data == NULL)
		return EXIT_FAILURE;
	if (msg == NULL)
		return EXIT_FAILURE;

	if (setjmp(data->env) == 0) {
		lua_getglobal(data->lua, "handle");
		lua_pushlightuserdata(data->lua, (void*)msg);
		return luaH_check_error(data->lua, lua_pcall(data->lua, 1, 0, 0));
	} else {
		lua_atpanic(data->lua, NULL);
		syslog(LOG_CRIT, "LUA: %s", lua_tostring(data->lua, -1));
		lua_pop(data->lua, 1);
		return EXIT_FAILURE;
	}
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
	struct dst_lua_data_t * data;

	if (!config)
		return EXIT_FAILURE;
	if (!properties)
		return EXIT_FAILURE;
	if (config->data != NULL)
		return EXIT_FAILURE;

	data = (struct dst_lua_data_t *)malloc(sizeof(struct dst_lua_data_t));
	config->data = data;
	init_data(data);

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

	/* introduce proc data to Lua state*/
	lua_pushlightuserdata(lua, data);
	lua_setglobal(lua, "__PROC_DATA__");

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

/**
 * Cleans up.
 *
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
static int exit_proc(struct proc_config_t * config)
{
	struct dst_lua_data_t * data;

	if (config == NULL)
		return EXIT_FAILURE;
	if (config->data == NULL)
		return EXIT_SUCCESS;

	data = (struct dst_lua_data_t *)config->data;
	if (data->lua) {
		lua_close(data->lua);
		data->lua = NULL;
	}

	free(config->data);
	config->data = NULL;
	return EXIT_SUCCESS;
}

static int proc(struct proc_config_t * config)
{
	int rc;
	int fd_max;
	fd_set rfds;
	struct message_t msg;
	struct dst_lua_data_t * data;
	struct signalfd_siginfo signal_info;

	if (config == NULL)
		return EXIT_FAILURE;

	data = (struct dst_lua_data_t *)config->data;
	if (!data)
		return EXIT_FAILURE;

	while (1) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->rfd, &rfds);
		if (config->rfd > fd_max)
			fd_max = config->rfd;
		FD_SET(config->signal_fd, &rfds);
		if (config->signal_fd > fd_max)
			fd_max = config->signal_fd;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, NULL, NULL);
		if (rc < 0 && errno != EINTR) {
			syslog(LOG_ERR, "error in 'select': %s", strerror(errno));
			return EXIT_FAILURE;
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->signal_fd, &rfds)) {
			rc = read(config->signal_fd, &signal_info, sizeof(signal_info));
			if (rc < 0 || rc != sizeof(signal_info)) {
				syslog(LOG_ERR, "cannot read singal info");
				return EXIT_FAILURE;
			}

			if (signal_info.ssi_signo == SIGTERM)
				break;
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
					if (process_message(data, &msg) != EXIT_SUCCESS)
						return EXIT_FAILURE;
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

static void help(void)
{
	printf("\n");
	printf("dst_lua\n");
	printf("\n");
	printf("Executes a Lua script for every message it should receive.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  script : filename of the Lua script to execute.\n");
	printf("  DEBUG  : [optional] a combination of 'c', 'r' and 'l' for debugging purposes.\n");
	printf("           c : call, traces function calls\n");
	printf("           r : return, traces function returns\n");
	printf("           l : line, traces executed lines\n");
	printf("\n");
	printf("Example:\n");
	printf("  log : dst_lua { script='log.lua', period:1000 };\n");
	printf("\n");
	printf("Example of Lua script:\n");
	printf("\n");
	printf("  function handle(msg)\n");
	printf("     print(msg.msg_type)\n");
	printf("  end\n");
	printf("\n");
}

const struct proc_desc_t dst_lua = {
	.name = "dst_lua",
	.init = init_proc,
	.exit = exit_proc,
	.func = proc,
	.help = help,
};

