#include <cunit/CUnit.h>
#include <test_config.h>
#include <config/config.h>

static void test_register_source(void)
{
	int rc;

	rc = config_register_source("test");
	CU_ASSERT_EQUAL(rc, 0);

	config_register_free();
}

static void test_register_destination(void)
{
	int rc;

	rc = config_register_destination("test");
	CU_ASSERT_EQUAL(rc, 0);

	config_register_free();
}

static void test_register_filter(void)
{
	int rc;

	rc = config_register_filter("test");
	CU_ASSERT_EQUAL(rc, 0);

	config_register_free();
}

static void test_register_check_source(void)
{
	int rc;

	rc = config_register_source("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_source("test1");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_source("test");
	CU_ASSERT_EQUAL(rc, 1);

	rc = config_registered_as_destination("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_filter("test");
	CU_ASSERT_EQUAL(rc, 0);

	config_register_free();
}

static void test_register_check_destination(void)
{
	int rc;

	rc = config_register_destination("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_source("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_destination("test1");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_destination("test");
	CU_ASSERT_EQUAL(rc, 1);

	rc = config_registered_as_filter("test");
	CU_ASSERT_EQUAL(rc, 0);

	config_register_free();
}

static void test_register_check_filter(void)
{
	int rc;

	rc = config_register_filter("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_source("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_destination("test");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_filter("test1");
	CU_ASSERT_EQUAL(rc, 0);

	rc = config_registered_as_filter("test");
	CU_ASSERT_EQUAL(rc, 1);

	config_register_free();
}

static void test_parse_string(void)
{
	CU_FAIL(); /* TODO */
}

void register_suite_config(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("config", NULL, NULL);
	CU_add_test(suite, "register source", test_register_source);
	CU_add_test(suite, "register destination", test_register_destination);
	CU_add_test(suite, "register filter", test_register_filter);
	CU_add_test(suite, "register check source", test_register_check_source);
	CU_add_test(suite, "register check destination", test_register_check_destination);
	CU_add_test(suite, "register check filter", test_register_check_filter);
	CU_add_test(suite, "parse string", test_parse_string);
}

