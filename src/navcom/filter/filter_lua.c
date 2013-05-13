#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <common/fileutil.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

static void lua_pushchar(lua_State * lua, char val)
{
	char str[2];

	str[0] = val;
	str[1] = '\0';

	lua_pushstring(lua, str);
}

static void lua_pushnmeafix(lua_State * lua, const struct nmea_fix_t * a)
{
	double t;

	nmea_fix_to_double(&t, a);
	lua_pushnumber(lua, t);
}

static void lua_pushnmeatime(lua_State * lua, const struct nmea_time_t * t)
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

static void lua_pushnmeadate(lua_State * lua, const struct nmea_date_t * t)
{
	lua_newtable(lua);

	lua_pushunsigned(lua, t->y);
	lua_setfield(lua, -2, "y");

	lua_pushunsigned(lua, t->m);
	lua_setfield(lua, -2, "m");

	lua_pushunsigned(lua, t->d);
	lua_setfield(lua, -2, "d");
}

static void lua_pushnmeaangle(lua_State * lua, const struct nmea_angle_t * a)
{
	double t;

	nmea_angle_to_double(&t, a);
	lua_pushnumber(lua, t);
}

static void define_const(lua_State * lua, const char * sym, int val)
{
	lua_pushinteger(lua, val);
	lua_setglobal(lua, sym);
}

static void define_unsigned_const(lua_State * lua, const char * sym, uint32_t val)
{
	lua_pushunsigned(lua, val);
	lua_setglobal(lua, sym);
}

static void define_char_const(lua_State * lua, const char * sym, char val)
{
	lua_pushchar(lua, val);
	lua_setglobal(lua, sym);
}

/**
 * Returns a string to the Lua release.
 */
const char * filter_lua_release(void)
{
	return LUA_RELEASE;
}

/**
 * Writes the string by the Lua script to the syslog.
 *
 * Lua example:
 * @code
 * syslog(LOG_NOTICE, 'Message')
 * @endcode
 */
static int lua__syslog(lua_State * lua)
{
	int type = -1;
	const char * str = NULL;

	type = luaL_checkinteger(lua, -2);
	str = luaL_checkstring(lua, -1);

	syslog(type, "%s", str);
	return 0;
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

	lua_pushnmeatime(lua, &s->time);
	lua_setfield(lua, -2, "time");

	lua_pushchar(lua, s->status);
	lua_setfield(lua, -2, "status");

	lua_pushnmeaangle(lua, &s->lat);
	lua_setfield(lua, -2, "lat");

	lua_pushchar(lua, s->lat_dir);
	lua_setfield(lua, -2, "lat_dir");

	lua_pushnmeaangle(lua, &s->lon);
	lua_setfield(lua, -2, "lon");

	lua_pushchar(lua, s->lon_dir);
	lua_setfield(lua, -2, "lon_dir");

	lua_pushnmeafix(lua, &s->sog);
	lua_setfield(lua, -2, "sog");

	lua_pushnmeafix(lua, &s->head);
	lua_setfield(lua, -2, "head");

	lua_pushnmeadate(lua, &s->date);
	lua_setfield(lua, -2, "date");

	lua_pushnmeafix(lua, &s->m);
	lua_setfield(lua, -2, "m");

	lua_pushchar(lua, s->m_dir);
	lua_setfield(lua, -2, "m_dir");

	lua_pushchar(lua, s->sig_integrity);
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
	define_const(lua, "FILTER_SUCCESS", FILTER_SUCCESS);
	define_const(lua, "FILTER_FAILURE", FILTER_FAILURE);
	define_const(lua, "FILTER_DISCARD", FILTER_DISCARD);
}

static void setup_syslog(lua_State * lua)
{
	define_const(lua, "LOG_CRIT",    LOG_CRIT);
	define_const(lua, "LOG_ERR",     LOG_ERR);
	define_const(lua, "LOG_WARNING", LOG_WARNING);
	define_const(lua, "LOG_DEBUG",   LOG_DEBUG);
	define_const(lua, "LOG_NOTICE",  LOG_NOTICE);

	lua_register(lua, "syslog", lua__syslog);
}

static void setup_message_handling(lua_State * lua)
{
	/* message types */
	define_unsigned_const(lua, "MSG_SYSTEM", MSG_SYSTEM);
	define_unsigned_const(lua, "MSG_TIMER",  MSG_TIMER);
	define_unsigned_const(lua, "MSG_NMEA",   MSG_NMEA);

	/* system message types */
	define_unsigned_const(lua, "SYSTEM_TERMINATE", SYSTEM_TERMINATE);

	/* nmea sentences */
	define_unsigned_const(lua, "NMEA_NONE",       NMEA_NONE);
	define_unsigned_const(lua, "NMEA_RMB",        NMEA_RMB);
	define_unsigned_const(lua, "NMEA_RMC",        NMEA_RMC);
	define_unsigned_const(lua, "NMEA_GGA",        NMEA_GGA);
	define_unsigned_const(lua, "NMEA_GSA",        NMEA_GSA);
	define_unsigned_const(lua, "NMEA_GSV",        NMEA_GSV);
	define_unsigned_const(lua, "NMEA_GLL",        NMEA_GLL);
	define_unsigned_const(lua, "NMEA_RTE",        NMEA_RTE);
	define_unsigned_const(lua, "NMEA_VTG",        NMEA_VTG);
	define_unsigned_const(lua, "NMEA_BOD",        NMEA_BOD);
	define_unsigned_const(lua, "NMEA_GARMIN_RME", NMEA_GARMIN_RME);
	define_unsigned_const(lua, "NMEA_GARMIN_RMM", NMEA_GARMIN_RMM);
	define_unsigned_const(lua, "NMEA_GARMIN_RMZ", NMEA_GARMIN_RMZ);
	define_unsigned_const(lua, "NMEA_HC_HDG",     NMEA_HC_HDG);

	/* nmea directions */
	define_char_const(lua, "NMEA_EAST",  NMEA_EAST);
	define_char_const(lua, "NMEA_WEST",  NMEA_WEST);
	define_char_const(lua, "NMEA_NORTH", NMEA_NORTH);
	define_char_const(lua, "NEMA_SOUGH", NEMA_SOUGH);

	/* nmea status */
	define_char_const(lua, "NMEA_STATUS_OK",      NMEA_STATUS_OK);
	define_char_const(lua, "NMEA_STATUS_WARNING", NMEA_STATUS_WARNING);

	/* message functions */
	lua_register(lua, "msg_clone", lua__msg_clone);
	lua_register(lua, "msg_type", lua__msg_type);
	lua_register(lua, "msg_to_table", lua__msg_to_table);
}

static void stacktrace(lua_State * lua)
{
	lua_Debug entry;
	int depth = 0;

	while (lua_getstack(lua, depth, &entry)) {
		lua_getinfo(lua, "Sln", &entry);
		printf("%s:%d: %s\n",
			entry.short_src,
			entry.currentline,
			entry.name ? entry.name : "?");
		++depth;
	}
}

/**
 * Writes debug information to the syslog.
 */
static void debug_hook(lua_State * lua, lua_Debug * debug)
{
	switch (debug->event) {
		case LUA_HOOKCALL:
			lua_getinfo(lua, "Sln", debug);
			syslog(LOG_DEBUG, "debug: call '%s'",
				debug->name ? debug->name : "?");
			break;

		case LUA_HOOKRET:
			lua_getinfo(lua, "Sln", debug);
			syslog(LOG_DEBUG, "debug: return '%s' at %d",
				debug->name ? debug->name : "?",
				debug->currentline);
			break;

		case LUA_HOOKLINE:
			lua_getinfo(lua, "l", debug);
			syslog(LOG_DEBUG, "debug: line %d",
				debug->currentline);
			break;
	}
}

/**
 * Sets up the debugging hook according to the property.
 *
 * The properties value may be a string containing the combination
 * of the characters 'c' (call), 'r' (return) and 'l' (line).
 *
 * If none of the flags is set, the debug hook will not be set.
 *
 * It is not possible to remove the debug hook during runtime.
 */
static void setup_debug(lua_State * lua, const struct property_t * debug_property)
{
	int mask = 0;
	const char * p;

	if (debug_property == NULL) return;
	for (p = debug_property->value; p && *p; ++p) {
		switch (tolower(*p)) {
			case 'c': mask |= LUA_MASKCALL; break;
			case 'r': mask |= LUA_MASKRET;  break;
			case 'l': mask |= LUA_MASKLINE; break;
			default: break;
		}
	}

	if (!mask) return;

	lua_sethook(lua, debug_hook, mask, 0);
}

static int setup_lua_state(lua_State * lua, const struct property_t * debug_property)
{
	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);

	setup_filter_results(lua);
	setup_syslog(lua);
	setup_message_handling(lua);
	setup_debug(lua, debug_property);

	/* TODO: setup error handling */

	return 0;
}

static int configure(
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	int rc;
	lua_State * lua = NULL;
	const struct property_t * prop = NULL;
	const struct property_t * debug_property = NULL;

	prop = proplist_find(properties, "script");
	if (!prop) {
		syslog(LOG_ERR, "no script defined");
		return FILTER_FAILURE;
	}

	if (!file_is_readable(prop->value)) {
		syslog(LOG_ERR, "file not readable: '%s'", prop->value);
		return FILTER_FAILURE;
	}

	debug_property = proplist_find(properties, "DEBUG");

	/* setup lua state */
	lua = luaL_newstate();
	if (!lua) {
		syslog(LOG_ERR, "unable to create lua state");
		return FILTER_FAILURE;
	}
	if (setup_lua_state(lua, debug_property)) {
		syslog(LOG_ERR, "unable to setup lua state");
		return FILTER_FAILURE;
	}

	/* load/execute script */
	rc = luaL_dofile(lua, prop->value);
	if (rc) {
		syslog(LOG_ERR, "unable to load and execute script: '%s'", prop->value);
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

