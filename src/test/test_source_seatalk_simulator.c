#include <cunit/CUnit.h>
#include <test_source_seatalk_simulator.h>
#include <navcom/source/seatalk_simulator.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &seatalk_simulator;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NULL(proc->help);
}

/**
 * @todo Check resuls of after init
 */
static void test_init(void)
{
	struct proc_config_t config;
	struct property_list_t properties;

	proc_config_init(&config);
	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(&config, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	/* period */
	proplist_set(&properties, "period", "zzz");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "period", "1000");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	/* depth */
	proplist_set(&properties, "depth", "zzz");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "depth", "30");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_source_seatalk_simulator(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/seatalk_simulator", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "init", test_init);
}

