#include <cunit/CUnit.h>
#include <test_source_gps_simulator.h>
#include <navcom/source/gps_simulator.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &gps_simulator;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->exit);
}

/**
 * @todo Check resuls of after init
 */
static void test_init(void)
{
	struct property_list_t properties;

	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* period */
	proplist_set(&properties, "period", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "period", "1000");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* sog */
	proplist_set(&properties, "sog", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "sog", "123");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* heading */
	proplist_set(&properties, "heading", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "heading", "123");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* mag */
	proplist_set(&properties, "mag", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "mag", "3");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* date */
	proplist_set(&properties, "date", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "date", "2013-05-22");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* time */
	proplist_set(&properties, "time", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "time", "12-34");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* lat */
	proplist_set(&properties, "lat", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "lat", "12-34,5N");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* lon */
	proplist_set(&properties, "lon", "xyz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "lon", "123-45,6N");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	/* simulated */
	proplist_set(&properties, "__not_simulated__", "");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_source_gps_simulator(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/gps_simulator", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "init", test_init);
}

