#include <cunit/CUnit.h>
#include <cunit/Basic.h>
#include <stdlib.h>

static void test(void)
{
	CU_ASSERT(1);
}

static int setup(void)
{
	return 0;
}

static int cleanup(void)
{
	return 0;
}

int main()
{
	CU_Suite * suite;

	CU_initialize_registry();

	suite = CU_add_suite("test suite", setup, cleanup);
	CU_add_test(suite, "test", test);

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return EXIT_SUCCESS;
}

