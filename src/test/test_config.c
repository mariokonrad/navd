#include <cunit/CUnit.h>
#include <test_config.h>
#include <config/config.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

static char tmpfilename[PATH_MAX];
static int fd = -1;

static int setup(void)
{
	strncpy(tmpfilename, "/tmp/test_configXXXXXXXX", sizeof(tmpfilename));
	fd = mkstemp(tmpfilename);
	return fd < 0;
}

static int cleanup(void)
{
	close(fd);
	unlink(tmpfilename);
	return 0;
}

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

static void test_parse_file_failures(void)
{
	int rc;
	struct config_t config;

	config_init(&config);
	rc = config_parse_file(NULL, NULL);
	CU_ASSERT_EQUAL(rc, -1);
	config_free(&config);

	config_init(&config);
	rc = config_parse_file(NULL, &config);
	CU_ASSERT_EQUAL(rc, -1);
	config_free(&config);

	config_init(&config);
	rc = config_parse_file("", NULL);
	CU_ASSERT_EQUAL(rc, -1);
	config_free(&config);

	config_register_free();
}

static void test_parse_file_source(void)
{
	int rc;
	struct config_t config;

	const char CONFIG[] =
		"a : src {};\n"
		;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);
	rc = write(fd, CONFIG, strlen(CONFIG));
	CU_ASSERT(rc == strlen(CONFIG));

	rc = config_register_source("src");
	config_init(&config);
	rc = config_parse_file(tmpfilename, &config);
	config_free(&config);
	config_register_free();

	CU_ASSERT_EQUAL(rc, 0);
}

static void test_parse_file_destination(void)
{
	int rc;
	struct config_t config;

	const char CONFIG[] =
		"a : dst {};\n"
		;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);
	rc = write(fd, CONFIG, strlen(CONFIG));
	CU_ASSERT(rc == strlen(CONFIG));

	rc = config_register_destination("dst");
	config_init(&config);
	rc = config_parse_file(tmpfilename, &config);
	config_free(&config);
	config_register_free();

	CU_ASSERT_EQUAL(rc, 0);
}

static void test_parse_file_filter(void)
{
	int rc;
	struct config_t config;

	const char CONFIG[] =
		"a : flt {};\n"
		;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);
	rc = write(fd, CONFIG, strlen(CONFIG));
	CU_ASSERT(rc == strlen(CONFIG));

	rc = config_register_filter("flt");
	config_init(&config);
	rc = config_parse_file(tmpfilename, &config);
	config_free(&config);
	config_register_free();

	CU_ASSERT_EQUAL(rc, 0);
}

static void test_parse_file_source_properties(void)
{
	int rc;
	struct config_t config;

	const char CONFIG[] =
		"a : src { enable baud=9600 };\n"
		;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);
	rc = write(fd, CONFIG, strlen(CONFIG));
	CU_ASSERT(rc == strlen(CONFIG));

	rc = config_register_source("src");
	config_init(&config);
	rc = config_parse_file(tmpfilename, &config);
	config_free(&config);
	config_register_free();

	CU_ASSERT_EQUAL(rc, 0);
}

static void test_parse_file(void)
{
	int rc;
	struct config_t config;

	const char CONFIG[] =
		"a  : src { enable baud=9600 };\n"
		"b0 : dst { file=test.txt };\n"
		"b1 : dst { file=test.txt };\n"
		"b2 : dst { file=test.txt };\n"
		"b3 : dst { file=test.txt };\n"
		"b4 : dst { file=test.txt };\n"
		"b5 : dst { file=test.txt };\n"
		"b6 : dst { file=test.txt };\n"
		"b7 : dst { file=test.txt };\n"
		"c  : flt { GPS };\n"
		"a -> b0;\n"
		"a -> b1;\n"
		"a -> [c] -> b2;\n"
		"a -> (b3 b4);\n"
		"a -> [c] -> ( b5 b6 );\n"
		;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);
	rc = write(fd, CONFIG, strlen(CONFIG));
	CU_ASSERT(rc == strlen(CONFIG));

	rc = config_register_source("src");
	CU_ASSERT_EQUAL(rc, 0);
	rc = config_register_destination("dst");
	CU_ASSERT_EQUAL(rc, 0);
	rc = config_register_filter("flt");
	CU_ASSERT_EQUAL(rc, 0);
	config_init(&config);
	rc = config_parse_file(tmpfilename, &config);
	config_free(&config);
	config_register_free();

	CU_ASSERT_EQUAL(rc, 0);

	CU_ASSERT_EQUAL(config.num_sources, 1);
	CU_ASSERT_EQUAL(config.num_destinations, 8);
	CU_ASSERT_EQUAL(config.num_filters, 1);
	CU_ASSERT_EQUAL(config.num_routes, 7);
}

void register_suite_config(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("config", setup, cleanup);
	CU_add_test(suite, "register source", test_register_source);
	CU_add_test(suite, "register destination", test_register_destination);
	CU_add_test(suite, "register filter", test_register_filter);
	CU_add_test(suite, "register check source", test_register_check_source);
	CU_add_test(suite, "register check destination", test_register_check_destination);
	CU_add_test(suite, "register check filter", test_register_check_filter);
	CU_add_test(suite, "parse file failures", test_parse_file_failures);
	CU_add_test(suite, "parse file source", test_parse_file_source);
	CU_add_test(suite, "parse file destination", test_parse_file_destination);
	CU_add_test(suite, "parse file filter", test_parse_file_filter);
	CU_add_test(suite, "parse file source properties", test_parse_file_source_properties);
	CU_add_test(suite, "parse file", test_parse_file);
}

