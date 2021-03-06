#include <cunit/CUnit.h>
#include <test_destination_nmea_serial.h>
#include <navcom/destination/nmea_serial.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &nmea_serial;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->exit);
}

/**
 * @todo Check for values after 'init'
 */
static void test_init(void)
{
	struct property_list_t properties;
	struct proc_config_t config;

	proc_config_init(&config);
	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(&config, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "device", "/dev/null");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "baud", "zzz");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "baud", "9600");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "parity", "zzz");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "parity", "none");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "data", "z");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "data", "8");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "stop", "z");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_set(&properties, "stop", "1");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_FAILURE);
}

void register_suite_destination_nmea_serial(void)
{
	char name[128];
	CU_Suite * suite;

	snprintf(name, sizeof(name), "destination/%s", proc->name);

	suite = CU_add_suite(name, NULL, NULL);
	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "exit", test_exit);
	CU_add_test(suite, "init", test_init);
}

