#include <cunit/CUnit.h>
#include <test_source_timer.h>
#include <navcom/source/timer.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &timer;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->configure);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
}

static void test_configure(void)
{
	struct property_list_t properties;
	int rc;

	proplist_init(&properties);

	rc = proc->configure(NULL, NULL);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	rc = proc->configure(NULL, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	proplist_set(&properties, "id", "1");

	rc = proc->configure(NULL, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	proplist_set(&properties, "period", "100");

	rc = proc->configure(NULL, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_set(&properties, "id", "xyz");

	rc = proc->configure(NULL, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	proplist_set(&properties, "id", "1");
	proplist_set(&properties, "period", "xyz");

	rc = proc->configure(NULL, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	proplist_free(&properties);
}

void register_suite_source_timer(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/timer", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "configure", test_configure);
}

