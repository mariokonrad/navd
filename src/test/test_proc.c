#include <cunit/CUnit.h>
#include <test_proc.h>
#include <navcom/proc.h>
#include <common/macros.h>
#include <string.h>

static void test_init(void)
{
	struct proc_config_t cfg;

	memset(&cfg, 0, sizeof(cfg));
	proc_config_init(&cfg);

	CU_ASSERT_EQUAL(cfg.pid, -1);
	CU_ASSERT_EQUAL(cfg.rfd, -1);
	CU_ASSERT_EQUAL(cfg.wfd, -1);
	CU_ASSERT_EQUAL(cfg.cfg, NULL);
	CU_ASSERT_EQUAL(cfg.data, NULL);
}

void register_suite_proc(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("proc", NULL, NULL);
	CU_add_test(suite, "init", test_init);
}

