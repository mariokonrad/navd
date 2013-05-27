#ifndef __NAVCOM__DST_LUA__H__
#define __NAVCOM__DST_LUA__H__

#include <navcom/proc.h>

struct lua_State;
typedef struct lua_State lua_State;

struct dst_lua_data_t
{
	lua_State * lua;
};

extern const struct proc_desc_t dst_lua;
extern const char * dst_lua_release(void);

#endif
