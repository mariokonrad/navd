#include <cunit/CUnit.h>
#include <test_source_gps_serial.h>
#include <navcom/source/gps_serial.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &gps_serial;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_source_gps_serial(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/gps_serial", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

