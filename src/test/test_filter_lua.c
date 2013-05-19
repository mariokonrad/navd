#include <cunit/CUnit.h>
#include <test_filter_lua.h>
#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

static const struct filter_desc_t * filter = &filter_lua;
static char tmpfilename[PATH_MAX];
static int fd = -1;

static void prepare_script(const char * code)
{
	int rc;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);

	rc = write(fd, code, strlen(code));
	CU_ASSERT_EQUAL(rc, (int)strlen(code));

	rc = lseek(fd, SEEK_SET, 0);
	CU_ASSERT_EQUAL(rc, 0);
}

static int setup(void)
{
	strncpy(tmpfilename, "/tmp/test_filter_luaXXXXXXXX", sizeof(tmpfilename));
	fd = mkstemp(tmpfilename);
	return fd < 0;
}

static int cleanup(void)
{
	close(fd);
	unlink(tmpfilename);
	return 0;
}

static void test_configure(void)
{
	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->configure, NULL);
}

static void test_free_ctx(void)
{
	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->free_ctx, NULL);
}

static void test_configure_free(void)
{
	struct filter_context_t ctx;
	struct property_list_t properties;
	int rc;

	const char SCRIPT[] = "\n";

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->configure, NULL);

	proplist_init(&properties);

	CU_ASSERT_EQUAL(filter->configure(NULL, NULL), FILTER_FAILURE);
	CU_ASSERT_EQUAL(filter->configure(NULL, &properties), FILTER_FAILURE);
	CU_ASSERT_EQUAL(filter->configure(&ctx, NULL), FILTER_FAILURE);
	CU_ASSERT_EQUAL(filter->configure(&ctx, &properties), FILTER_FAILURE);

	proplist_set(&properties, "script", "");

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_FAILURE);

	prepare_script(SCRIPT);

	proplist_set(&properties, "script", tmpfilename);
	CU_ASSERT_EQUAL(filter->configure(&ctx, &properties), FILTER_SUCCESS);
	CU_ASSERT_NOT_EQUAL(ctx.data, NULL);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_debug(void)
{
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;
	int rc;

	const char SCRIPT[] =
		"function barfoo()\n"
		"	return FILTER_DISCARD\n"
		"end\n"
		"\n"
		"function foobar()\n"
		"	barfoo()\n"
		"	return FILTER_DISCARD\n"
		"end\n"
		"\n"
		"function filter(mo, mi)\n"
		"	foobar()\n"
		"	return FILTER_SUCCESS\n"
		"end\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->configure, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	prepare_script(SCRIPT);
	proplist_set(&properties, "script", tmpfilename);
	proplist_set(&properties, "DEBUG", "crl");

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_syslog(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(mout, min)\n"
		"	syslog(LOG_NOTICE, 'executing filter: testing syslog')\n"
		"	return FILTER_DISCARD\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_DISCARD);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_clone(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	return msg_clone(msg_out, msg_in)\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	msg_in.type = MSG_TIMER;
	msg_in.data.timer_id = 12345678;

	rc = filter->func(&msg_out, NULL, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(NULL, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(msg_out.type, msg_in.type);
	CU_ASSERT_EQUAL(msg_out.data.timer_id, msg_in.data.timer_id);

	CU_ASSERT_EQUAL(msg_in.type, MSG_TIMER);
	CU_ASSERT_EQUAL(msg_in.data.timer_id, 12345678);

	CU_ASSERT_EQUAL(msg_out.type, MSG_TIMER);
	CU_ASSERT_EQUAL(msg_out.data.timer_id, 12345678);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_type(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	if msg_type(msg_in) == MSG_NMEA then\n"
		"		return FILTER_SUCCESS\n"
		"	end\n"
		"	return FILTER_FAILURE\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	rc = filter->func(&msg_out, NULL, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	msg_in.type = MSG_SYSTEM;

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	msg_in.type = MSG_NMEA;

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_to_table(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	local t = msg_to_table(msg_in)\n"
		"   if t == nil then\n"
		"		return FILTER_DISCARD\n"
		"	end\n"
		"	return FILTER_SUCCESS\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	rc = filter->func(&msg_out, NULL, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	msg_in.type = MSG_SYSTEM;

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_to_table_system(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	local t = msg_to_table(msg_in)\n"
		"   if t == nil then\n"
		"		return FILTER_DISCARD\n"
		"	end\n"
		"	if t.msg_type == MSG_SYSTEM then\n"
		"		if t.data.system == SYSTEM_TERMINATE then\n"
		"			return FILTER_SUCCESS\n"
		"		end\n"
		"	end\n"
		"	return FILTER_FAILURE\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	msg_in.type = MSG_SYSTEM;
	msg_in.data.system = SYSTEM_TERMINATE;

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_to_table_timer(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	local t = msg_to_table(msg_in)\n"
		"   if t == nil then\n"
		"		return FILTER_DISCARD\n"
		"	end\n"
		"	if t.msg_type == MSG_TIMER then\n"
		"		if t.data.timer_id == 12345678 then\n"
		"			return FILTER_SUCCESS\n"
		"		end\n"
		"	end\n"
		"	return FILTER_FAILURE\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	msg_in.type = MSG_TIMER;
	msg_in.data.timer_id = 12345678;

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_to_table_nmea(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	local t = msg_to_table(msg_in)\n"
		"   if t == nil then\n"
		"		return FILTER_DISCARD\n"
		"	end\n"
		"	if t.msg_type == MSG_NMEA then\n"
		"		if t.data.nmea.nmea_type == NMEA_RMB then\n"
		"			if t.data.nmea.raw == 'hello world' then\n"
		"				return FILTER_SUCCESS\n"
		"			end\n"
		"		end\n"
		"	end\n"
		"	return FILTER_FAILURE\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	msg_in.type = MSG_NMEA;
	msg_in.data.nmea.type = NMEA_RMB;
	strncpy(msg_in.data.nmea.raw, "hello world", sizeof(msg_in.data.nmea.raw));

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

static void test_func_msg_to_table_nmea_rmc(void)
{
	int rc;
	struct filter_context_t ctx;
	struct property_list_t properties;
	struct message_t msg_in;
	struct message_t msg_out;

	const char SCRIPT[] =
		"function filter(msg_out, msg_in)\n"
		"	local t = msg_to_table(msg_in)\n"
		"   if t == nil then\n"
		"		return FILTER_DISCARD\n"
		"	end\n"
		"	if t.msg_type ~= MSG_NMEA then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.nmea_type ~= NMEA_RMC then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.status ~= NMEA_STATUS_OK then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.lat ~= 1.5 then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.lat_dir ~= NMEA_NORTH then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.lon ~= 1.5 then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.lon_dir ~= NMEA_WEST then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.sog ~= 5.0 then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	if t.data.nmea.sentence.head ~= 10.0 then\n"
		"		return FILTER_FAILURE\n"
		"	end\n"
		"	return FILTER_SUCCESS\n"
		"end\n"
		"\n"
		;

	memset(&msg_in, 0, sizeof(msg_in));
	memset(&msg_out, 0, sizeof(msg_out));

	CU_ASSERT_NOT_EQUAL(filter, NULL);
	CU_ASSERT_NOT_EQUAL(filter->func, NULL);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = filter->configure(&ctx, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, FILTER_SUCCESS);

	msg_in.type = MSG_NMEA;
	msg_in.data.nmea.type = NMEA_RMC;
	msg_in.data.nmea.sentence.rmc.status = NMEA_STATUS_OK;
	msg_in.data.nmea.sentence.rmc.lat.d = 1;
	msg_in.data.nmea.sentence.rmc.lat.m = 30;
	msg_in.data.nmea.sentence.rmc.lat.s.i = 0;
	msg_in.data.nmea.sentence.rmc.lat.s.d = 0;
	msg_in.data.nmea.sentence.rmc.lat_dir = NMEA_NORTH;
	msg_in.data.nmea.sentence.rmc.lon.d = 1;
	msg_in.data.nmea.sentence.rmc.lon.m = 30;
	msg_in.data.nmea.sentence.rmc.lon.s.i = 0;
	msg_in.data.nmea.sentence.rmc.lon.s.d = 0;
	msg_in.data.nmea.sentence.rmc.lon_dir = NMEA_WEST;
	msg_in.data.nmea.sentence.rmc.sog.i = 5;
	msg_in.data.nmea.sentence.rmc.sog.d = 0;
	msg_in.data.nmea.sentence.rmc.head.i = 10;
	msg_in.data.nmea.sentence.rmc.head.d = 0;

	rc = filter->func(&msg_out, &msg_in, &ctx, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

void register_suite_filter_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, setup, cleanup);

	CU_add_test(suite, "configure", test_configure);
	CU_add_test(suite, "free_ctx", test_free_ctx);
	CU_add_test(suite, "configure / free", test_configure_free);
	CU_add_test(suite, "debug", test_debug);
	CU_add_test(suite, "func: syslog", test_func_syslog);
	CU_add_test(suite, "func: msg_clone", test_func_msg_clone);
	CU_add_test(suite, "func: msg_type", test_func_msg_type);
	CU_add_test(suite, "func: msg_to_table", test_func_msg_to_table);
	CU_add_test(suite, "func: msg_to_table: system", test_func_msg_to_table_system);
	CU_add_test(suite, "func: msg_to_table: timer", test_func_msg_to_table_timer);
	CU_add_test(suite, "func: msg_to_table: nmea", test_func_msg_to_table_nmea);
	CU_add_test(suite, "func: msg_to_table: nmea: RMC", test_func_msg_to_table_nmea_rmc);
}

