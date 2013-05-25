#ifndef __NAVCOM__LUA_NMEA__H__
#define __NAVCOM__LUA_NMEA__H__

#include <lua/lua.h>

struct nmea_fix_t;
struct nmea_angle_t;
struct nmea_time_t;
struct nmea_date_t;

void luaH_pushnmeafix(lua_State *, const struct nmea_fix_t *);
void luaH_pushnmeatime(lua_State *, const struct nmea_time_t *);
void luaH_pushnmeadate(lua_State *, const struct nmea_date_t *);
void luaH_pushnmeaangle(lua_State *, const struct nmea_angle_t *);

void luaH_checknmeafix(lua_State *, int, struct nmea_fix_t *);
void luaH_checknmeaangle(lua_State *, int, struct nmea_angle_t *);

#endif
