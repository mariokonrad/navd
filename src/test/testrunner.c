#include <cunit/CUnit.h>
#include <cunit/Basic.h>
#include <stdlib.h>
#include <test_strlist.h>
#include <test_property.h>
#include <test_nmea.h>
#include <test_config.h>
#include <test_filter_null.h>
#include <test_filter_nmea.h>
#include <test_filter_lua.h>
#include <test_filterlist.h>
#include <test_device.h>
#include <test_property_serial.h>
#include <test_fileutil.h>

int main()
{
	CU_initialize_registry();

	register_suite_strlist();
	register_suite_property();
	register_suite_nmea();
	register_suite_config();
	register_suite_filter_null();
	register_suite_filter_nmea();
	register_suite_filter_lua();
	register_suite_filterlist();
	register_suite_device();
	register_suite_property_serial();
	register_suite_fileutil();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return EXIT_SUCCESS;
}

