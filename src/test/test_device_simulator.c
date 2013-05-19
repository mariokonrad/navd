#include <cunit/CUnit.h>
#include <test_device_simulator.h>
#include <device/simulator.h>
#include <common/macros.h>
#include <string.h>

static const struct device_operations_t * device = &simulator_operations;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_device_simulator(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("device/simulator", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

