#include <cunit/CUnit.h>
#include <cunit/Basic.h>
#include <stdlib.h>
#include <test_strlist.h>
#include <test_property.h>
#include <test_nmea.h>
#include <test_config.h>

int main()
{
	CU_initialize_registry();

	register_suite_strlist();
	register_suite_property();
	register_suite_nmea();
	register_suite_config();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return EXIT_SUCCESS;
}

