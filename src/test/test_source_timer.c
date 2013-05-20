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
	CU_ASSERT_PTR_NOT_NULL(proc->clean);
}

/**
 * @todo Check for values after 'configure'
 */
static void test_configure(void)
{
	struct property_list_t properties;
	struct proc_config_t config;

	proc_config_init(&config);
	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->configure(NULL, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->configure(NULL, &properties), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->configure(&config, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->clean(&config), EXIT_SUCCESS);

	CU_ASSERT_EQUAL(proc->configure(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->clean(&config), EXIT_SUCCESS);

	proplist_set(&properties, "id", "1");
	CU_ASSERT_EQUAL(proc->configure(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->clean(&config), EXIT_SUCCESS);

	proplist_set(&properties, "period", "100");
	CU_ASSERT_EQUAL(proc->configure(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->clean(&config), EXIT_SUCCESS);

	proplist_set(&properties, "id", "xyz");
	CU_ASSERT_EQUAL(proc->configure(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->clean(&config), EXIT_SUCCESS);

	proplist_set(&properties, "id", "1");
	proplist_set(&properties, "period", "xyz");
	CU_ASSERT_EQUAL(proc->configure(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->clean(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_clean(void)
{
	CU_ASSERT_EQUAL(proc->clean(NULL), EXIT_FAILURE);
}

void register_suite_source_timer(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/timer", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "configure", test_configure);
	CU_add_test(suite, "clean", test_clean);
}

