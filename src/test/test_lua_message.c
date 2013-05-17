#include <cunit/CUnit.h>
#include <test_lua_message.h>
#include <navcom/lua_message.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

static int call(const char * script, struct message_t * msg)
{
	lua_State * lua;
	int rc = -1;

	lua = luaL_newstate();
	CU_ASSERT_PTR_NOT_NULL_FATAL(lua);
	if (!lua) return -1;

	luaopen_base(lua);
	luaopen_table(lua);
	luaopen_string(lua);
	luaopen_math(lua);
	luaH_setup_message_handling(lua);

	rc = luaL_dostring(lua, script);
	CU_ASSERT_EQUAL(rc, LUA_OK);
	if (rc != LUA_OK) {
		printf("\nlua error: %s\n", lua_tostring(lua, -1));
	}

	memset(msg, 0, sizeof(struct message_t));

	lua_getglobal(lua, "handle");
	lua_pushlightuserdata(lua, (void*)msg);
	CU_ASSERT_EQUAL(lua_pcall(lua, 1, 1, 0), LUA_OK);
	if (rc != LUA_OK) {
		printf("\nlua error: %s\n", lua_tostring(lua, -1));
	} else {
		rc = luaL_checkinteger(lua, -1);
	}

	lua_close(lua);

	return rc;
}

static void test_msg_type(void)
{
	const char * SCRIPT =
		"function handle(msg_out)\n"
		"	msg_from_table(msg_out, { type = MSG_SYSTEM })\n"
		"	return 1\n"
		"end\n"
		;

	int rc;
	struct message_t msg;

	rc = call(SCRIPT, &msg);

	CU_ASSERT_EQUAL(rc, 1);
	CU_ASSERT_EQUAL(msg.type, MSG_SYSTEM);
}

static void test_msg_system_type(void)
{
	const char * SCRIPT =
		"function handle(msg_out)\n"
		"	msg_from_table(msg_out, { type = MSG_SYSTEM, data = { system = SYSTEM_TERMINATE } })\n"
		"	return 1\n"
		"end\n"
		;

	int rc;
	struct message_t msg;

	rc = call(SCRIPT, &msg);

	CU_ASSERT_EQUAL(rc, 1);
	CU_ASSERT_EQUAL(msg.type, MSG_SYSTEM);
	CU_ASSERT_EQUAL(msg.data.system, SYSTEM_TERMINATE);
}

static void test_msg_timer_id(void)
{
	const char * SCRIPT =
		"function handle(msg_out)\n"
		"	msg_from_table(msg_out, { type = MSG_TIMER, data = { timer_id = 1234 } })\n"
		"	return 1\n"
		"end\n"
		;

	int rc;
	struct message_t msg;

	rc = call(SCRIPT, &msg);

	CU_ASSERT_EQUAL(rc, 1);
	CU_ASSERT_EQUAL(msg.type, MSG_TIMER);
	CU_ASSERT_EQUAL(msg.data.timer_id, 1234);
}

void register_suite_lua_message(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("lua_message", NULL, NULL);

	CU_add_test(suite, "message type", test_msg_type);
	CU_add_test(suite, "message system type", test_msg_system_type);
	CU_add_test(suite, "message timer id", test_msg_timer_id);
	/* TODO: add NMEA tests */
}

