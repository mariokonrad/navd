#include <cunit/CUnit.h>
#include <test_destination_logbook.h>
#include <navcom/destination/logbook.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &logbook;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->exit);
}

static void test_init(void)
{
	struct property_list_t properties;

	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_save_timer_id(void)
{
	struct property_list_t properties;

	proplist_init(&properties);
	proplist_set(&properties, "filename", "/dev/null");
	proplist_set(&properties, "write_timeout", "5");
	proplist_set(&properties, "min_position_change", "5");

	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "save_timer_id", "");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "save_timer_id", "zzz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "save_timer_id", "5");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_filename(void)
{
	struct property_list_t properties;

	proplist_init(&properties);
	proplist_set(&properties, "save_timer_id", "5");
	proplist_set(&properties, "write_timeout", "5");
	proplist_set(&properties, "min_position_change", "5");

	proplist_set(&properties, "filename", "");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "filename", "/dev/null");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_write_timeout(void)
{
	struct property_list_t properties;

	proplist_init(&properties);
	proplist_set(&properties, "save_timer_id", "5");
	proplist_set(&properties, "filename", "/dev/null");
	proplist_set(&properties, "min_position_change", "5");

	proplist_set(&properties, "write_timeout", "");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "write_timeout", "zzz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "write_timeout", "5");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_min_position_change(void)
{
	struct property_list_t properties;

	proplist_init(&properties);
	proplist_set(&properties, "save_timer_id", "5");
	proplist_set(&properties, "write_timeout", "5");
	proplist_set(&properties, "filename", "/dev/null");

	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "min_position_change", "");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "min_position_change", "zzz");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "min_position_change", "-5");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_set(&properties, "min_position_change", "5");
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_SUCCESS);
}

void register_suite_destination_logbook(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/logbook", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "init: save_timer_id", test_init_save_timer_id);
	CU_add_test(suite, "init: filename", test_init_filename);
	CU_add_test(suite, "init: write_timeout", test_init_write_timeout);
	CU_add_test(suite, "init: min_position_change", test_init_min_position_change);
	CU_add_test(suite, "exit", test_exit);
}

