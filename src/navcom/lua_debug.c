#include <navcom/lua_debug.h>
#include <syslog.h>
#include <ctype.h>

/**
 * Dumps the contents of the stack.
 *
 * @todo Dump top of stack first
 */
void luaH_stacktrace(lua_State * lua)
{
	int i;
	int top = lua_gettop(lua);
	printf("stack trace:\n");
	for (i = 1; i <= top; ++i) {
		int t = lua_type(lua, i);
		printf("%3d: ", i);
		switch (t) {
			case LUA_TSTRING:
				printf("'%s'", lua_tostring(lua, i));
				break;

			case LUA_TBOOLEAN:
				printf(lua_toboolean(lua, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:
				printf("%g", lua_tonumber(lua, i));
				break;

			default:
				printf("%s", lua_typename(lua, t));
				break;

		}
		printf("\n");
	}
	printf("\n");
}

/**
 * Writes debug information to the syslog.
 */
static void debug_hook(lua_State * lua, lua_Debug * debug)
{
	switch (debug->event) {
		case LUA_HOOKCALL:
			lua_getinfo(lua, "Sln", debug);
			syslog(LOG_DEBUG, "debug: call '%s'",
				debug->name ? debug->name : "?");
			break;

		case LUA_HOOKRET:
			lua_getinfo(lua, "Sln", debug);
			syslog(LOG_DEBUG, "debug: return '%s' at %d",
				debug->name ? debug->name : "?",
				debug->currentline);
			break;

		case LUA_HOOKLINE:
			lua_getinfo(lua, "l", debug);
			syslog(LOG_DEBUG, "debug: line %d",
				debug->currentline);
			break;
	}
}

/**
 * Sets up the debugging hook according to the property.
 *
 * The properties value may be a string containing the combination
 * of the characters 'c' (call), 'r' (return) and 'l' (line).
 *
 * If none of the flags is set, the debug hook will not be set.
 *
 * It is not possible to remove the debug hook during runtime.
 */
void luaH_setup_debug(lua_State * lua, const struct property_t * debug_property)
{
	int mask = 0;
	const char * p;

	if (debug_property == NULL) return;
	for (p = debug_property->value; p && *p; ++p) {
		switch (tolower(*p)) {
			case 'c': mask |= LUA_MASKCALL; break;
			case 'r': mask |= LUA_MASKRET;  break;
			case 'l': mask |= LUA_MASKLINE; break;
			default: break;
		}
	}

	if (!mask) return;

	lua_sethook(lua, debug_hook, mask, 0);
}

