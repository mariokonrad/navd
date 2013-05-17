#ifndef __NAVCOM__LUA_HELPER__H__
#define __NAVCOM__LUA_HELPER__H__

#include <lua/lua.h>
#include <common/property.h>

void luaH_define_const(lua_State *, const char *, int);
void luaH_define_unsigned_const(lua_State *, const char *, uint32_t);
void luaH_define_char_const(lua_State *, const char *, char);
void luaH_pushchar(lua_State *, char);
int luaH_checkscript_from_prop(const struct property_t *);

#endif
