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
	CU_ASSERT_PTR_NOT_NULL(proc->configure);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
}

static void test_configure(void)
{
	struct proc_t info;
	struct proc_config_t config;
	int rc;

	memset(&info, 0, sizeof(info));
	info.name = "name";
	info.name = "type";
	proplist_init(&info.properties);
	proplist_set(&info.properties, "enable", "true");
	proplist_set(&info.properties, "dst", "/dev/null");

	proc_config_init(&config);
	config.cfg = &info;

	rc = proc->configure(&config, &info.properties);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&info.properties);
}

void register_suite_destination_message_log(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/message_log", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "configure", test_configure);
}

