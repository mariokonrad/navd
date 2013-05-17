#include <navcom/filter/filter_lua.h>
#include <navcom/common/lua_helper.h>
#include <navcom/common/lua_syslog.h>
#include <navcom/common/lua_debug.h>
#include <navcom/common/lua_nmea.h>
#include <common/macros.h>
#include <common/fileutil.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

/**
 * Returns a string to the Lua release.
 */
const char * filter_lua_release(void)
{
	return LUA_RELEASE;
}

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
		lua_pushinteger(lua, FILTER_FAILURE);
		return 1;
	}

	memcpy(msg_out, msg_in, sizeof(struct message_t));

	lua_pushinteger(lua, FILTER_SUCCESS);
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

static void msg_table_system(lua_State * lua, const struct message_t * msg)
{
	lua_pushunsigned(lua, msg->data.system);
	lua_setfield(lua, -2, "system");
}

static void msg_table_timer(lua_State * lua, const struct message_t * msg)
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

static void msg_table_nmea(lua_State * lua, const struct message_t * msg)
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
	lua_setfield(lua, -2, "type");

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
		{ MSG_SYSTEM, msg_table_system },
		{ MSG_TIMER,  msg_table_timer  },
		{ MSG_NMEA,   msg_table_nmea   },
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
	lua_setfield(lua, -2, "type");

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

static void setup_filter_results(lua_State * lua)
{
	luaH_define_const(lua, "FILTER_SUCCESS", FILTER_SUCCESS);
	luaH_define_const(lua, "FILTER_FAILURE", FILTER_FAILURE);
	luaH_define_const(lua, "FILTER_DISCARD", FILTER_DISCARD);
}

static void setup_message_handling(lua_State * lua)
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
}

static int setup_lua_state(lua_State * lua, const struct property_t * debug_property)
{
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);

	luaH_setup_syslog(lua);
	luaH_setup_debug(lua, debug_property);
	setup_filter_results(lua);
	setup_message_handling(lua);

	/* TODO: setup error handling */

	return EXIT_SUCCESS;
}

static int configure(
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;
	const struct property_t * prop_script = NULL;
	const struct property_t * prop_debug = NULL;

	prop_script = proplist_find(properties, "script");
	prop_debug = proplist_find(properties, "DEBUG");

	rc = luaH_checkscript_from_prop(prop_script);
	if (rc != EXIT_SUCCESS) return rc;

	/* setup lua state */
	lua = luaL_newstate();
	if (!lua) {
		syslog(LOG_ERR, "unable to create lua state");
		return FILTER_FAILURE;
	}
	if (setup_lua_state(lua, prop_debug)) {
		syslog(LOG_ERR, "unable to setup lua state");
		return FILTER_FAILURE;
	}

	/* load/execute script */
	rc = luaL_dofile(lua, prop_script->value);
	if (rc) {
		syslog(LOG_ERR, "unable to load and execute script: '%s'", prop_script->value);
		lua_close(lua);
		return FILTER_FAILURE;
	}

	ctx->data = lua;
	return FILTER_SUCCESS;
}

int free_context(struct filter_context_t * ctx)
{
	if (ctx && ctx->data) {
		lua_State * lua = (lua_State *)ctx->data;
		lua_close(lua);
		ctx->data = NULL;
	}

	return FILTER_SUCCESS;
}

static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;

	UNUSED_ARG(out);
	UNUSED_ARG(in);
	UNUSED_ARG(properties);

	if (!ctx || !ctx->data) return FILTER_FAILURE;
	lua = (lua_State *)ctx->data;

	lua_getglobal(lua, "filter");
	lua_pushlightuserdata(lua, (void*)out);
	lua_pushlightuserdata(lua, (void*)in);
	rc = lua_pcall(lua, 2, 1, 0);
	if (rc != LUA_OK) {
		const char * err_string = lua_tostring(lua, -1);
		switch (rc) {
			case LUA_ERRRUN:
				syslog(LOG_ERR, "runtime error: '%s'", err_string);
				break;
			case LUA_ERRMEM:
				syslog(LOG_ERR, "memory allocation error: '%s'", err_string);
				break;
			case LUA_ERRERR:
				syslog(LOG_ERR, "message handler error: '%s'", err_string);
				break;
			case LUA_ERRGCMM:
				syslog(LOG_ERR, "error while calling metamethod: '%s'", err_string);
				break;
		}
		lua_pop(lua, 1);
		return FILTER_FAILURE;
	}

	return luaL_checkinteger(lua, -1);
}

const struct filter_desc_t filter_lua = {
	.name = "filter_lua",
	.configure = configure,
	.free_ctx = free_context,
	.func = filter
};

