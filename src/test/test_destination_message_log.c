#include <cunit/CUnit.h>
#include <test_destination_message_log.h>
#include <navcom/destination/message_log.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &message_log;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_destination_message_log(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/message_log", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

