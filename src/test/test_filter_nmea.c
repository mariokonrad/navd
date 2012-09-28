#include <cunit/CUnit.h>
#include <test_filter_nmea.h>
#include <navcom/filter/filter_nmea.h>
#include <common/macros.h>

static const struct filter_desc_t * filter = &filter_nmea;

static void test_configure(void)
{
	CU_FAIL();
}

static void test_free_ctx(void)
{
	CU_FAIL();
}

static void test_func(void)
{
	CU_FAIL();
}

void register_suite_filter_nmea(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, NULL, NULL);

	CU_add_test(suite, "configure", test_configure);
	CU_add_test(suite, "free_ctx", test_free_ctx);
	CU_add_test(suite, "func", test_func);
}

