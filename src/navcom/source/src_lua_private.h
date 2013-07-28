#ifndef __NAVCOM__SRC_LUA_PRIVATE__H__
#define __NAVCOM__SRC_LUA_PRIVATE__H__

#include <lua/lua.h>
#include <setjmp.h>
#include <sys/time.h>

struct src_lua_data_t
{
	lua_State * lua;
	jmp_buf env;
	int initialized;
	struct timeval tm_cfg;
};

#endif
