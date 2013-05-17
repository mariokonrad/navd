#include <navcom/common/lua_syslog.h>
#include <navcom/common/lua_helper.h>
#include <syslog.h>
#include <lua/lauxlib.h>

/**
 * Writes the string by the Lua script to the syslog.
 *
 * Lua example:
 * @code
 * syslog(LOG_NOTICE, 'Message')
 * @endcode
 */
static int lua__syslog(lua_State * lua)
{
	int type = -1;
	const char * str = NULL;

	type = luaL_checkinteger(lua, -2);
	str = luaL_checkstring(lua, -1);

	syslog(type, "%s", str);
	return 0;
}

void luaH_setup_syslog(lua_State * lua)
{
	luaH_define_const(lua, "LOG_CRIT",    LOG_CRIT);
	luaH_define_const(lua, "LOG_ERR",     LOG_ERR);
	luaH_define_const(lua, "LOG_WARNING", LOG_WARNING);
	luaH_define_const(lua, "LOG_DEBUG",   LOG_DEBUG);
	luaH_define_const(lua, "LOG_NOTICE",  LOG_NOTICE);

	lua_register(lua, "syslog", lua__syslog);
}

