#include <cunit/CUnit.h>
#include <test_source_gps_simulator.h>
#include <navcom/source/gps_simulator.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &gps_simulator;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_source_gps_simulator(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/gps_simulator", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

