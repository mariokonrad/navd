#include <navcom/lua_message.h>
#include <navcom/message.h>
#include <navcom/lua_helper.h>
#include <navcom/lua_nmea.h>
#include <navcom/lua_debug.h>
#include <stdlib.h>
#include <string.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

/**
 * Clones the message.
 *
 * Lua example:
 * @code
 * function filter(msg_out, msg_in)
 *     local rc = msg_clone(msg_out, msg_in)
 *     return FILTER_SUCCESS
 * end
 * @endcode
 */
static int lua__msg_clone(lua_State * lua)
{
	struct message_t * msg_out;
	struct message_t * msg_in;

	msg_out = lua_touserdata(lua, -2);
	msg_in = lua_touserdata(lua, -1);

	if (!msg_out || !msg_in) {
		lua_pushinteger(lua, EXIT_FAILURE);
		return 1;
	}

	memcpy(msg_out, msg_in, sizeof(struct message_t));

	lua_pushinteger(lua, EXIT_SUCCESS);
	return 1;
}

/**
 * Returns the message type. The resulting type is unsigned.
 *
 * Lua example:
 * @code
 * function filter(msg_out, msg_in)
 *     local t = msg_type(msg_in)
 *     return FILTER_SUCCESS
 * end
 * @endcode
 */
static int lua__msg_type(lua_State * lua)
{
	struct message_t * msg;

	msg = lua_touserdata(lua, -1);
	if (msg == NULL) {
		lua_pushunsigned(lua, MSG_INVALID);
		return 1;
	}
	lua_pushunsigned(lua, msg->type);
	return 1;
}

static void msg_to_table_system(lua_State * lua, const struct message_t * msg)
{
	lua_pushunsigned(lua, msg->data.system);
	lua_setfield(lua, -2, "system");
}

static void msg_to_table_timer(lua_State * lua, const struct message_t * msg)
{
	lua_pushunsigned(lua, msg->data.timer_id);
	lua_setfield(lua, -2, "timer_id");
}

static void msg_table_nmea_rmc(lua_State * lua, const struct nmea_t * nmea)
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
static void msg_to_table_nmea(lua_State * lua, const struct message_t * msg)
{
	struct msg_nmea_t
	{
		uint32_t type;
		void (*func)(lua_State *, const struct nmea_t *);
	};

	static const struct msg_nmea_t CONV[] =
	{
		{ NMEA_RMC, msg_table_nmea_rmc },
	};

	const struct nmea_t * nmea = &msg->data.nmea;
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

/**
 * Converts the speficied message to a table.
 *
 * Currently supported message types:
 * - MSG_SYSTEM
 * - MSG_TIMER
 * - MSG_NMEA (partially)
 *
 * Lua example:
 * @code
 * function filter(msg_out, msg_in)
 *     local t = msg_to_table(msg_in)
 *     if t.type == MSG_SYSTEM then
 *         return FILTER_SUCCESS
 *     end
 *     return FILTER_FAILURE
 * end
 * @endcode
 */
static int lua__msg_to_table(lua_State * lua)
{
	struct msg_conversion_t
	{
		uint32_t type;
		void (*func)(lua_State *, const struct message_t *);
	};

	static const struct msg_conversion_t CONV[] =
	{
		{ MSG_SYSTEM, msg_to_table_system },
		{ MSG_TIMER,  msg_to_table_timer  },
		{ MSG_NMEA,   msg_to_table_nmea   },
	};

	struct message_t * msg;
	size_t i;

	msg = lua_touserdata(lua, -1);
	if (msg == NULL) {
		lua_pushnil(lua);
		return 1;
	}

	lua_newtable(lua);

	lua_pushunsigned(lua, msg->type);
	lua_setfield(lua, -2, "msg_type");

	lua_newtable(lua);
	for (i = 0; i < sizeof(CONV) / sizeof(CONV[0]); ++i) {
		if ((CONV[i].type == msg->type) && CONV[i].func) {
			CONV[i].func(lua, msg);
			break;
		}
	}
	lua_setfield(lua, -2, "data");

	return 1;
}

/**
 * @todo Implement error handling
 */
static int msg_from_table_system(lua_State * lua, struct message_t * msg)
{
	lua_getfield(lua, -1, "system");
	msg->data.system = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	return 0;
}

/**
 * Converts the table from the Lua state to the specified message.
 *
 * Lua example:
 * @code
 * function handle(msg_out)
 *     local t = { ... }
 *     local rc = msg_from_table(msg_out, t)
 *     return 0
 * end
 * @endcode
 */
static int lua__msg_from_table(lua_State * lua)
{
	struct msg_conversion_t
	{
		uint32_t type;
		int (*func)(lua_State *, struct message_t *);
	};

	static const struct msg_conversion_t CONV[] =
	{
		{ MSG_SYSTEM, msg_from_table_system },
		/* TODO: { MSG_SYSTEM, msg_from_table_timer  }, */
		/* TODO: { MSG_SYSTEM, msg_from_table_nmea   }, */
	};

	struct message_t * msg;
	size_t i;
	int rc = 0;

	msg = lua_touserdata(lua, -2);
	if (msg == NULL) {
		lua_pushinteger(lua, 1);
		return 1;
	}

	/* type */
	lua_getfield(lua, -1, "msg_type");
	msg->type = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);

	/* data */
	lua_getfield(lua, -1, "data");
	for (i = 0; i < sizeof(CONV) / sizeof(CONV[0]); ++i) {
		if (msg->type == CONV[i].type) {
			rc = CONV[i].func(lua, msg);
			break;
		}
	}
	lua_pop(lua, 1);

	/* result */
	lua_pushinteger(lua, rc);
	return 1;
}

void luaH_setup_message_handling(lua_State * lua)
{
	/* message types */
	luaH_define_unsigned_const(lua, "MSG_SYSTEM", MSG_SYSTEM);
	luaH_define_unsigned_const(lua, "MSG_TIMER",  MSG_TIMER);
	luaH_define_unsigned_const(lua, "MSG_NMEA",   MSG_NMEA);

	/* system message types */
	luaH_define_unsigned_const(lua, "SYSTEM_TERMINATE", SYSTEM_TERMINATE);

	/* nmea sentences */
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

	/* message functions */
	lua_register(lua, "msg_clone", lua__msg_clone);
	lua_register(lua, "msg_type", lua__msg_type);
	lua_register(lua, "msg_to_table", lua__msg_to_table);
	lua_register(lua, "msg_from_table", lua__msg_from_table);
}

