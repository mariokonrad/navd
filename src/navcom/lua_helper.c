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

