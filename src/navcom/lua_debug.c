#include <navcom/lua_debug.h>
#include <syslog.h>
#include <ctype.h>

void luaH_stacktrace(lua_State * lua)
{
	lua_Debug entry;
	int depth = 0;

	while (lua_getstack(lua, depth, &entry)) {
		lua_getinfo(lua, "Sln", &entry);
		printf("%s:%d: %s\n",
			entry.short_src,
			entry.currentline,
			entry.name ? entry.name : "?");
		++depth;
	}
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

