#ifndef __NAVCOM__LUA_DEBUG__H__
#define __NAVCOM__LUA_DEBUG__H__

#include <lua/lua.h>
#include <common/property.h>

void luaH_stacktrace(lua_State *);
void luaH_setup_debug(lua_State *, const struct property_t *);

#endif
