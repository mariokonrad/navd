#include <navcom/lua_message.h>
#include <navcom/message.h>
#include <navcom/lua_helper.h>
#include <navcom/lua_debug.h>
#include <global_config.h>
#include <stdlib.h>
#include <string.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

#if defined(NEEDS_NMEA)
	#include <navcom/lua_message_nmea.h>
#endif

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

	if ((msg_out == NULL) || (msg_in == NULL)) {
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
	lua_pushunsigned(lua, msg->data.attr.system);
	lua_setfield(lua, -2, "system");
}

static void msg_to_table_timer(lua_State * lua, const struct message_t * msg)
{
	lua_pushunsigned(lua, msg->data.attr.timer_id);
	lua_setfield(lua, -2, "timer_id");
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
 *     if t.msg_type == MSG_SYSTEM then
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
#if defined(NEEDS_NMEA)
		{ MSG_NMEA,   luaH_msg_to_table_nmea },
#endif
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
 * Reads a system message from lua state.
 *
 * @retval EXIT_SUCCESS
 */
static int msg_from_table_system(lua_State * lua, struct message_t * msg)
{
	lua_getfield(lua, -1, "system");
	msg->data.attr.system = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	return EXIT_SUCCESS;
}

/**
 * Reads a timer message from lua state.
 *
 * @retval EXIT_SUCCESS
 */
static int msg_from_table_timer(lua_State * lua, struct message_t * msg)
{
	lua_getfield(lua, -1, "timer_id");
	msg->data.attr.timer_id = luaL_checkunsigned(lua, -1);
	lua_pop(lua, 1);
	return EXIT_SUCCESS;
}

/**
 * Converts the table from the Lua state to the specified message.
 *
 * Lua example:
 * @code
 * function handle(msg_out)
 *     local t = { ... }
 *     local rc = msg_from_table(msg_out, t)
 *     return rc
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
		{ MSG_TIMER,  msg_from_table_timer  },
#if defined(NEEDS_NMEA)
		{ MSG_NMEA,   luaH_msg_from_table_nmea },
#endif
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
	luaH_define_unsigned_const(lua, "MSG_SYSTEM",  MSG_SYSTEM);
	luaH_define_unsigned_const(lua, "MSG_TIMER",   MSG_TIMER);
	luaH_define_unsigned_const(lua, "MSG_NMEA",    MSG_NMEA);
	luaH_define_unsigned_const(lua, "MSG_SEATALK", MSG_SEATALK);

	/* system message types */
	luaH_define_unsigned_const(lua, "SYSTEM_TERMINATE", SYSTEM_TERMINATE);

	/* message functions */
	lua_register(lua, "msg_clone", lua__msg_clone);
	lua_register(lua, "msg_type", lua__msg_type);
	lua_register(lua, "msg_to_table", lua__msg_to_table);
	lua_register(lua, "msg_from_table", lua__msg_from_table);

	luaH_setup_message_nmea_handling(lua);
}

