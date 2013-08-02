#ifndef __NAVCOM__LUA_MESSAGE_NMEA__H__
#define __NAVCOM__LUA_MESSAGE_NMEA__H__

#include <lua/lua.h>

struct message_t;

void luaH_setup_message_nmea_handling(lua_State *);
void luaH_msg_to_table_nmea(lua_State *, const struct message_t *);
int luaH_msg_from_table_nmea(lua_State *, struct message_t *);

#endif
