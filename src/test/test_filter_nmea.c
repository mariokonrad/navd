#include <cunit/CUnit.h>
#include <test_filter_nmea.h>
#include <navcom/filter/filter_nmea.h>
#include <common/macros.h>

static const struct filter_desc_t * filter = &filter_nmea;
static struct property_list_t proplist;

static int setup(void)
{
	if (proplist_init(&proplist) != 0) return -1;
	return 0;
}

static int cleanup(void)
{
	if (proplist_free(&proplist) != 0) return -1;
	return 0;
}

static void test_init(void)
{
	CU_ASSERT_EQUAL(filter->init, NULL);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(filter->exit, NULL);
}

static void test_func_parameter(void)
{
	int rc;
	struct message_t out;
	struct message_t in;

	rc = filter->func(NULL, NULL, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(&out, NULL, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(NULL, &in, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(NULL, NULL, NULL, &proplist);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);
}

static void test_func_unsupported(void)
{
	int rc;
	struct message_t out;
	struct message_t in;

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_SYSTEM;
	rc = filter->func(&out, &in, NULL, &proplist);
	CU_ASSERT_EQUAL(rc, FILTER_DISCARD);

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_TIMER;
	rc = filter->func(&out, &in, NULL, &proplist);
	CU_ASSERT_EQUAL(rc, FILTER_DISCARD);
}

static void test_func_invalid(void)
{
	int rc;
	struct message_t out;
	struct message_t in;

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_NMEA;
	in.data.attr.nmea.type = NMEA_NONE;
	rc = filter->func(&out, &in, NULL, &proplist);
	CU_ASSERT_EQUAL(rc, FILTER_DISCARD);
}

static void test_func(void)
{
	int rc;
	struct message_t out;
	struct message_t in;

	/* emtpy property list, filter of all nmea sentences */

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_NMEA;
	in.data.attr.nmea.type = NMEA_RMC;
	rc = filter->func(&out, &in, NULL, &proplist);
	CU_ASSERT_EQUAL(rc, FILTER_DISCARD);

	/* supported and specified nmea sentences */

	CU_ASSERT_EQUAL(proplist_append(&proplist, "GPRMB", NULL), 0);
	CU_ASSERT_EQUAL(proplist_append(&proplist, "GPRMC", NULL), 0);
	CU_ASSERT_EQUAL(proplist_append(&proplist, "GPGGA", NULL), 0);

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_NMEA;
	in.data.attr.nmea.type = NMEA_RMC;
	rc = filter->func(&out, &in, NULL, &proplist);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(memcmp(&out, &in, sizeof(out)), 0);
}

void register_suite_filter_nmea(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, setup, cleanup);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "exit", test_exit);
	CU_add_test(suite, "func: parameter", test_func_parameter);
	CU_add_test(suite, "func: unsupported message types", test_func_unsupported);
	CU_add_test(suite, "func: invalid message", test_func_invalid);
	CU_add_test(suite, "func", test_func);
}

