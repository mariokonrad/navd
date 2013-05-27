#ifndef __NAVCOM__DST_LUA_PRIVATE__H__
#define __NAVCOM__DST_LUA_PRIVATE__H__

#include <lua/lua.h>
#include <setjmp.h>

struct dst_lua_data_t
{
	lua_State * lua;
	jmp_buf env;
};

#endif
