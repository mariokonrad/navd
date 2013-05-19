#include <cunit/CUnit.h>
#include <test_destination_logbook.h>
#include <navcom/destination/logbook.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &logbook;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_destination_logbook(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/logbook", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

