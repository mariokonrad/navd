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

void register_suite_filter_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, setup, cleanup);

	CU_add_test(suite, "configure", test_configure);
	CU_add_test(suite, "free_ctx", test_free_ctx);
	CU_add_test(suite, "configure / free", test_configure_free);
	CU_add_test(suite, "func: syslog", test_func_syslog);
	CU_add_test(suite, "func: msg_clone", test_func_msg_clone);
	CU_add_test(suite, "func: msg_type", test_func_msg_type);
}

