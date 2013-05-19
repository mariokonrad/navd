#include <cunit/CUnit.h>
#include <test_source_timer.h>
#include <navcom/source/timer.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &timer;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_source_timer(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/timer", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

