#include <navcom/lua_helper.h>
#include <common/fileutil.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <syslog.h>
#include <stdlib.h>

void luaH_define_const(lua_State * lua, const char * sym, int val)
{
	lua_pushinteger(lua, val);
	lua_setglobal(lua, sym);
}

void luaH_define_unsigned_const(lua_State * lua, const char * sym, uint32_t val)
{
	lua_pushunsigned(lua, val);
	lua_setglobal(lua, sym);
}

void luaH_define_char_const(lua_State * lua, const char * sym, char val)
{
	luaH_pushchar(lua, val);
	lua_setglobal(lua, sym);
}

/**
 * Pushes the specified character as string onto the Lua stack.
 */
void luaH_pushchar(lua_State * lua, char val)
{
	char str[2];

	str[0] = val;
	str[1] = '\0';

	lua_pushstring(lua, str);
}

char luaH_checkchar(lua_State * lua, int index)
{
	const char * s;

	s = luaL_checkstring(lua, index);
	return s[0];
}

/**
 * Checks according to the specified property if the Lua script
 * is readable.
 *
 * @param[in] property Contains information about the script to execute.
 * @retval EXIT_SUCCESS Success.
 * @retval EXIT_FAILURE Failure.
 */
int luaH_checkscript_from_prop(const struct property_t * property)
{
	if (!property) {
		syslog(LOG_ERR, "no script defined");
		return EXIT_FAILURE;
	}

	if (!file_is_readable(property->value)) {
		syslog(LOG_ERR, "file not readable: '%s'", property->value);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Checks for Lua errors and logs them in the syslog.
 *
 * @param[out] lua The Lua state.
 * @param[in] error The error code to check.
 */
void luaH_check_error(lua_State * lua, int error)
{
	const char * err_string;

	if (error == LUA_OK)
		return;

	err_string = lua_tostring(lua, -1);
	switch (error) {
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
		case LUA_ERRSYNTAX:
			syslog(LOG_ERR, "syntax error: '%s'", err_string);
			break;
	}
	lua_pop(lua, 1);
}

