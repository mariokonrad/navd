#include <cunit/CUnit.h>
#include <test_source_gps_serial.h>
#include <navcom/source/gps_serial.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &gps_serial;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->configure);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->clean);
}

static void test_configure(void)
{
	struct property_list_t properties;

	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->configure(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_FAILURE);

	proplist_set(&properties, "device", "/dev/null");
	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_FAILURE);

	proplist_set(&properties, "baud", "9600");
	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_FAILURE);

	proplist_set(&properties, "parity", "none");
	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_FAILURE);

	proplist_set(&properties, "data", "8");
	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_FAILURE);

	proplist_set(&properties, "stop", "1");
	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_source_gps_serial(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/gps_serial", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "configure", test_configure);
}

