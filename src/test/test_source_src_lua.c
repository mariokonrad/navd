#include <cunit/CUnit.h>
#include <test_source_src_lua.h>
#include <navcom/source/src_lua.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &src_lua;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_source_src_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/src_lua", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

