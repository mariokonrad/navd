#include <cunit/CUnit.h>
#include <test_lua_message.h>
#include <navcom/lua_message.h>
#include <navcom/lua_debug.h>
#include <navcom/message.h>
#include <common/macros.h>
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include <setjmp.h>

static jmp_buf env;

static int panic(lua_State * lua)
{
	luaH_stacktrace(lua);
	longjmp(env, -1);
	return 0;
}

static int call(const char * script, struct message_t * msg)
{
	lua_State * lua;
	int rc = -1;

	lua = luaL_newstate();
	CU_ASSERT_PTR_NOT_NULL_FATAL(lua);
	if (!lua) return -1;

	lua_atpanic(lua, panic);
	luaL_openlibs(lua);
	luaH_setup_message_handling(lua);

	if (setjmp(env) == 0) {
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
	} else {
		lua_atpanic(lua, NULL);
		printf("LUA ERROR: %s\n", lua_tostring(lua, -1));
		lua_pop(lua, 1);
	}

	lua_close(lua);

	return rc;
}

static int call_write_read(
		const char * script,
		struct message_t * msg_out,
		struct message_t * msg_in)
{
	lua_State * lua;
	int rc = -1;

	lua = luaL_newstate();
	CU_ASSERT_PTR_NOT_NULL_FATAL(lua);
	if (!lua) return -1;

	lua_atpanic(lua, panic);
	luaL_openlibs(lua);
	luaH_setup_message_handling(lua);

	if (setjmp(env) == 0) {
		rc = luaL_dostring(lua, script);
		CU_ASSERT_EQUAL(rc, LUA_OK);
		if (rc != LUA_OK) {
			printf("\nlua error: %s\n", lua_tostring(lua, -1));
		}

		lua_getglobal(lua, "process");
		lua_pushlightuserdata(lua, (void*)msg_out);
		lua_pushlightuserdata(lua, (void*)msg_in);
		CU_ASSERT_EQUAL(lua_pcall(lua, 2, 1, 0), LUA_OK);
		if (rc != LUA_OK) {
			printf("\nlua error: %s\n", lua_tostring(lua, -1));
		} else {
			rc = luaL_checkinteger(lua, -1);
		}
	} else {
		lua_atpanic(lua, NULL);
		printf("LUA ERROR: %s\n", lua_tostring(lua, -1));
		lua_pop(lua, 1);
	}

	lua_close(lua);

	return rc;
}

static void test_msg_system_type(void)
{
	const char * SCRIPT =
		"function handle(msg_out)\n"
		"	msg_from_table(msg_out, { msg_type = MSG_SYSTEM, data = { system = SYSTEM_TERMINATE }})\n"
		"	return 1\n"
		"end\n"
		;

	int rc;
	struct message_t msg;

	rc = call(SCRIPT, &msg);

	CU_ASSERT_EQUAL(rc, 1);
	CU_ASSERT_EQUAL(msg.type, MSG_SYSTEM);
	CU_ASSERT_EQUAL(msg.data.attr.system, SYSTEM_TERMINATE);
}

static void test_msg_timer_id(void)
{
	const char * SCRIPT =
		"function handle(msg_out)\n"
		"	msg_from_table(msg_out, { msg_type = MSG_TIMER, data = { timer_id = 1234 }})\n"
		"	return 1\n"
		"end\n"
		;

	int rc;
	struct message_t msg;

	rc = call(SCRIPT, &msg);

	CU_ASSERT_EQUAL(rc, 1);
	CU_ASSERT_EQUAL(msg.type, MSG_TIMER);
	CU_ASSERT_EQUAL(msg.data.attr.timer_id, 1234);
}

static void test_msg_nmea_rmc(void)
{
	const char * SCRIPT =
		"function handle(msg_out)\n"
		"	local rmc = {\n"
		"		time = { h = 12, m = 34, s = 56 },\n"
		"		status = 'A',\n"
		"		lat = 12.56805555556,\n" /* 12-34,5 */
		"		lat_dir = 'N',\n"
		"		lon = 123.7516666667,\n" /* 123-45,6 */
		"		lon_dir = 'E',\n"
		"		sog = 5.4,\n"
		"		head = 123.5,\n"
		"		date = { y = 2000, m = 1, d = 23 },\n"
		"		m = 2.5,\n"
		"		m_dir = 'E',\n"
		"		sig_integrity = 'S'\n"
		"	}\n"
		"	msg_from_table(msg_out, { msg_type = MSG_NMEA, data = { nmea = { nmea_type = NMEA_RMC, raw = '', sentence = rmc }}})\n"
		"	return 1\n"
		"end\n"
		;

	int rc;
	struct message_t msg;

	rc = call(SCRIPT, &msg);

	CU_ASSERT_EQUAL(rc, 1);
	CU_ASSERT_EQUAL(msg.type, MSG_NMEA);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.type, NMEA_RMC);
	CU_ASSERT_STRING_EQUAL(msg.data.attr.nmea.raw, "");
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.time.h, 12);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.time.m, 34);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.time.s, 56);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.time.ms, 0);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.status, NMEA_STATUS_OK);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lat.d, 12);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lat.m, 34);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lat.s.i, 5);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lat.s.d, 0);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lat_dir, NMEA_NORTH);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lon.d, 123);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lon.m, 45);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lon.s.i, 6);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lon.s.d, 0);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.lon_dir, NMEA_EAST);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.sog.i, 5);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.sog.d, 400000);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.head.i, 123);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.head.d, 500000);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.date.y, 2000);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.date.m, 1);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.date.d, 23);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.m.i, 2);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.m.d, 500000);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.m_dir, NMEA_EAST);
	CU_ASSERT_EQUAL(msg.data.attr.nmea.sentence.rmc.sig_integrity, 'S');
}

static void test_msg_wr_empty(void)
{
	const char * SCRIPT =
		"function process(msg_out, msg_in)\n"
		"	return msg_from_table(msg_out, msg_to_table(msg_in))\n"
		"end\n"
		;

	struct message_t msg_in;
	struct message_t msg_out;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_EQUAL(call_write_read(SCRIPT, &msg_out, &msg_in), 0);
	CU_ASSERT_EQUAL(memcmp(&msg_out, &msg_in, sizeof(msg_in)), 0);
}

static void test_msg_wr_system_terminate(void)
{
	const char * SCRIPT =
		"function process(msg_out, msg_in)\n"
		"	return msg_from_table(msg_out, msg_to_table(msg_in))\n"
		"end\n"
		;

	struct message_t msg_in;
	struct message_t msg_out;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	msg_in.type = MSG_SYSTEM;
	msg_in.data.attr.system = SYSTEM_TERMINATE;

	CU_ASSERT_EQUAL(call_write_read(SCRIPT, &msg_out, &msg_in), 0);
	CU_ASSERT_EQUAL(memcmp(&msg_out, &msg_in, sizeof(msg_in)), 0);
}

static void test_msg_wr_timer(void)
{
	const char * SCRIPT =
		"function process(msg_out, msg_in)\n"
		"	return msg_from_table(msg_out, msg_to_table(msg_in))\n"
		"end\n"
		;

	struct message_t msg_in;
	struct message_t msg_out;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	msg_in.type = MSG_TIMER;
	msg_in.data.attr.timer_id = 0xdeadbeef;

	CU_ASSERT_EQUAL(call_write_read(SCRIPT, &msg_out, &msg_in), 0);
	CU_ASSERT_EQUAL(memcmp(&msg_out, &msg_in, sizeof(msg_in)), 0);
}

static void test_msg_wr_nmea_rmc(void)
{
	const char * SCRIPT =
		"function process(msg_out, msg_in)\n"
		"	return msg_from_table(msg_out, msg_to_table(msg_in))\n"
		"end\n"
		;

	struct message_t msg_in;
	struct message_t msg_out;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	msg_in.type = MSG_NMEA;
	msg_in.data.attr.nmea.type = NMEA_RMC;
	msg_in.data.attr.nmea.sentence.rmc.time.h = 12;
	msg_in.data.attr.nmea.sentence.rmc.time.m = 34;
	msg_in.data.attr.nmea.sentence.rmc.time.s = 56;
	msg_in.data.attr.nmea.sentence.rmc.status = NMEA_STATUS_OK;
	msg_in.data.attr.nmea.sentence.rmc.lat.d = 12;
	msg_in.data.attr.nmea.sentence.rmc.lat.m = 34;
	msg_in.data.attr.nmea.sentence.rmc.lat.s.i = 57;
	msg_in.data.attr.nmea.sentence.rmc.lat_dir = NMEA_NORTH;
	msg_in.data.attr.nmea.sentence.rmc.lon.d = 123;
	msg_in.data.attr.nmea.sentence.rmc.lon.m = 45;
	msg_in.data.attr.nmea.sentence.rmc.lon.s.i = 58;
	msg_in.data.attr.nmea.sentence.rmc.lon_dir = NMEA_EAST;
	msg_in.data.attr.nmea.sentence.rmc.sog.i = 5;
	msg_in.data.attr.nmea.sentence.rmc.head.i = 123;
	msg_in.data.attr.nmea.sentence.rmc.date.y = 2345;
	msg_in.data.attr.nmea.sentence.rmc.date.m = 12;
	msg_in.data.attr.nmea.sentence.rmc.date.d = 23;
	msg_in.data.attr.nmea.sentence.rmc.m.i = 2;
	msg_in.data.attr.nmea.sentence.rmc.m_dir = NMEA_WEST;
	msg_in.data.attr.nmea.sentence.rmc.sig_integrity = NMEA_SIG_INT_SIMULATED;

	CU_ASSERT_EQUAL(call_write_read(SCRIPT, &msg_out, &msg_in), 0);
	CU_ASSERT_EQUAL(memcmp(&msg_out, &msg_in, sizeof(msg_in)), 0);
}

void register_suite_lua_message(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("lua_message", NULL, NULL);

	CU_add_test(suite, "message system type", test_msg_system_type);
	CU_add_test(suite, "message timer id", test_msg_timer_id);
	CU_add_test(suite, "message nmea RMC", test_msg_nmea_rmc);
	CU_add_test(suite, "message write/read emtpy", test_msg_wr_empty);
	CU_add_test(suite, "message write/read system terminate", test_msg_wr_system_terminate);
	CU_add_test(suite, "message write/read timer", test_msg_wr_timer);
	CU_add_test(suite, "message write/read nmea RMC", test_msg_wr_nmea_rmc);
}

