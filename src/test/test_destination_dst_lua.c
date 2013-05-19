#include <cunit/CUnit.h>
#include <test_destination_dst_lua.h>
#include <navcom/destination/dst_lua.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &dst_lua;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_destination_dst_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/dst_lua", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

