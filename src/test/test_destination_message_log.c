#include <cunit/CUnit.h>
#include <test_destination_message_log.h>
#include <navcom/destination/message_log.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

static const struct proc_desc_t * proc = &message_log;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->exit);
	CU_ASSERT_PTR_NULL(proc->help);
}

static void test_init(void)
{
	struct proc_t info;
	struct proc_config_t config;

	memset(&info, 0, sizeof(info));
	info.name = "name";
	info.name = "type";
	proplist_init(&info.properties);
	proplist_set(&info.properties, "enable", "true");
	proplist_set(&info.properties, "dst", "/dev/null");

	proc_config_init(&config);
	config.cfg = &info;

	CU_ASSERT_EQUAL(proc->init(&config, &info.properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&info.properties);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_FAILURE);
}

void register_suite_destination_message_log(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/message_log", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "exit", test_exit);
}

