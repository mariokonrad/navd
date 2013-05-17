#include <navcom/lua_nmea.h>
#include <nmea/nmea_fix.h>
#include <nmea/nmea_angle.h>
#include <nmea/nmea_date.h>
#include <nmea/nmea_time.h>

void luaH_pushnmeafix(lua_State * lua, const struct nmea_fix_t * a)
{
	double t;

	nmea_fix_to_double(&t, a);
	lua_pushnumber(lua, t);
}

void luaH_pushnmeatime(lua_State * lua, const struct nmea_time_t * t)
{
	lua_newtable(lua);

	lua_pushunsigned(lua, t->h);
	lua_setfield(lua, -2, "h");

	lua_pushunsigned(lua, t->m);
	lua_setfield(lua, -2, "m");

	lua_pushunsigned(lua, t->s);
	lua_setfield(lua, -2, "s");

	lua_pushunsigned(lua, t->ms);
	lua_setfield(lua, -2, "ms");
}

void luaH_pushnmeadate(lua_State * lua, const struct nmea_date_t * t)
{
	lua_newtable(lua);

	lua_pushunsigned(lua, t->y);
	lua_setfield(lua, -2, "y");

	lua_pushunsigned(lua, t->m);
	lua_setfield(lua, -2, "m");

	lua_pushunsigned(lua, t->d);
	lua_setfield(lua, -2, "d");
}

void luaH_pushnmeaangle(lua_State * lua, const struct nmea_angle_t * a)
{
	double t;

	nmea_angle_to_double(&t, a);
	lua_pushnumber(lua, t);
}

