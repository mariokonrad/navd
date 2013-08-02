#include <navcom/lua_message_nmea.h>
#include <navcom/lua_nmea.h>
#include <navcom/lua_helper.h>
#include <navcom/message.h>
#include <nmea/nmea.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <stdlib.h>
#include <string.h>

static void msg_to_table_nmea_rmc(
		lua_State * lua,
		const struct nmea_t * nmea)
{
	const struct nmea_rmc_t * s = &nmea->sentence.rmc;

	luaH_pushnmeatime(lua, &s->time);
	lua_setfield(lua, -2, "time");

	luaH_pushchar(lua, s->status);
	lua_setfield(lua, -2, "status");

	luaH_pushnmeaangle(lua, &s->lat);
	lua_setfield(lua, -2, "lat");

	luaH_pushchar(lua, s->lat_dir);
	lua_setfield(lua, -2, "lat_dir");

	luaH_pushnmeaangle(lua, &s->lon);
	lua_setfield(lua, -2, "lon");

	luaH_pushchar(lua, s->lon_dir);
	lua_setfield(lua, -2, "lon_dir");

	luaH_pushnmeafix(lua, &s->sog);
	lua_setfield(lua, -2, "sog");

	luaH_pushnmeafix(lua, &s->head);
	lua_setfield(lua, -2, "head");

	luaH_pushnmeadate(lua, &s->date);
	lua_setfield(lua, -2, "date");

	luaH_pushnmeafix(lua, &s->m);
	lua_setfield(lua, -2, "m");

	luaH_pushchar(lua, s->m_dir);
	lua_setfield(lua, -2, "m_dir");

	luaH_pushchar(lua, s->sig_integrity);
	lua_setfield(lua, -2, "sig_integrity");
}

/**
 * Fills the table within the Lua state with the NMEA data.
 * Supported NMEA sentences are:
 * - RMC
 */
void luaH_msg_to_table_nmea(lua_State * lua, const struct message_t * msg)
{
	struct msg_nmea_t
	{
		uint32_t type;
		void (*func)(lua_State *, const struct nmea_t *);
	};

	static const struct msg_nmea_t CONV[] =
	{
		{ NMEA_RMC, msg_to_table_nmea_rmc },
	};

	const struct nmea_t * nmea = &msg->data.attr.nmea;
	size_t i;

	lua_newtable(lua);

	lua_pushunsigned(lua, nmea->type);
	lua_setfield(lua, -2, "nmea_type");

	lua_pushstring(lua, nmea->raw);
	lua_setfield(lua, -2, "raw");

	lua_newtable(lua);
	for (i = 0; i < sizeof(CONV) / sizeof(CONV[0]); ++i) {
		if ((CONV[i].type == nmea->type) && CONV[i].func) {
			CONV[i].func(lua, nmea);
			break;
		}
	}
	lua_setfield(lua, -2, "sentence");

	lua_setfield(lua, -2, "nmea");
}

static void msg_from_table_nmea_rmc(lua_State * lua, struct nmea_t * nmea)
{
	struct nmea_rmc_t * s = &nmea->sentence.rmc;

	lua_getfield(lua, -1, "time");
	lua_getfield(lua, -1, "h");
	s->time.h = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	lua_getfield(lua, -1, "m");
	s->time.m = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	lua_getfield(lua, -1, "s");
	s->time.s = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "status");
	s->status = luaH_checkchar(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "lat");
	luaH_checknmeaangle(lua, -1, &s->lat);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "lat_dir");
	s->lat_dir = luaH_checkchar(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "lon");
	luaH_checknmeaangle(lua, -1, &s->lon);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "lon_dir");
	s->lon_dir = luaH_checkchar(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "sog");
	luaH_checknmeafix(lua, -1, &s->sog);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "head");
	luaH_checknmeafix(lua, -1, &s->head);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "date");
	lua_getfield(lua, -1, "y");
	s->date.y = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	lua_getfield(lua, -1, "m");
	s->date.m = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	lua_getfield(lua, -1, "d");
	s->date.d = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "m");
	luaH_checknmeafix(lua, -1, &s->m);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "m_dir");
	s->m_dir = luaH_checkchar(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "sig_integrity");
	s->sig_integrity = luaH_checkchar(lua, -1);
	lua_pop(lua, 1);
}

/**
 * Reads a nmeaa message from lua state.
 *
 * @retval EXIT_SUCCESS
 */
int luaH_msg_from_table_nmea(lua_State * lua, struct message_t * msg)
{
	struct msg_nmea_t
	{
		uint32_t type;
		void (*func)(lua_State *, struct nmea_t *);
	};

	static const struct msg_nmea_t CONV[] =
	{
		{ NMEA_RMC, msg_from_table_nmea_rmc },
	};

	struct nmea_t * nmea = &msg->data.attr.nmea;
	size_t i;

	lua_getfield(lua, -1, "nmea");

	lua_getfield(lua, -1, "nmea_type");
	nmea->type = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "raw");
	strncpy(nmea->raw, luaL_checkstring(lua, -1), sizeof(nmea->raw) - 1);
	lua_pop(lua, 1);

	lua_getfield(lua, -1, "sentence");
	for (i = 0; i < sizeof(CONV) / sizeof(CONV[0]); ++i) {
		if ((CONV[i].type == nmea->type) && CONV[i].func) {
			CONV[i].func(lua, nmea);
			break;
		}
	}
	lua_pop(lua, 2);
	return EXIT_SUCCESS;
}

void luaH_setup_message_nmea_handling(lua_State * lua)
{
	luaH_define_unsigned_const(lua, "NMEA_NONE",       NMEA_NONE);
	luaH_define_unsigned_const(lua, "NMEA_RMB",        NMEA_RMB);
	luaH_define_unsigned_const(lua, "NMEA_RMC",        NMEA_RMC);
	luaH_define_unsigned_const(lua, "NMEA_GGA",        NMEA_GGA);
	luaH_define_unsigned_const(lua, "NMEA_GSA",        NMEA_GSA);
	luaH_define_unsigned_const(lua, "NMEA_GSV",        NMEA_GSV);
	luaH_define_unsigned_const(lua, "NMEA_GLL",        NMEA_GLL);
	luaH_define_unsigned_const(lua, "NMEA_RTE",        NMEA_RTE);
	luaH_define_unsigned_const(lua, "NMEA_VTG",        NMEA_VTG);
	luaH_define_unsigned_const(lua, "NMEA_BOD",        NMEA_BOD);
	luaH_define_unsigned_const(lua, "NMEA_GARMIN_RME", NMEA_GARMIN_RME);
	luaH_define_unsigned_const(lua, "NMEA_GARMIN_RMM", NMEA_GARMIN_RMM);
	luaH_define_unsigned_const(lua, "NMEA_GARMIN_RMZ", NMEA_GARMIN_RMZ);
	luaH_define_unsigned_const(lua, "NMEA_HC_HDG",     NMEA_HC_HDG);

	/* nmea directions */
	luaH_define_char_const(lua, "NMEA_EAST",  NMEA_EAST);
	luaH_define_char_const(lua, "NMEA_WEST",  NMEA_WEST);
	luaH_define_char_const(lua, "NMEA_NORTH", NMEA_NORTH);
	luaH_define_char_const(lua, "NEMA_SOUGH", NEMA_SOUGH);

	/* nmea status */
	luaH_define_char_const(lua, "NMEA_STATUS_OK",      NMEA_STATUS_OK);
	luaH_define_char_const(lua, "NMEA_STATUS_WARNING", NMEA_STATUS_WARNING);
}

