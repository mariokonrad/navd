#include <global_config.h>
#include <cunit/CUnit.h>
#include <cunit/Basic.h>
#include <stdlib.h>
#include <test_strlist.h>
#include <test_property.h>
#include <test_nmea.h>
#include <test_seatalk.h>
#include <test_config.h>
#include <test_filter_null.h>
#include <test_filter_nmea.h>
#include <test_filterlist.h>
#include <test_device.h>
#include <test_device_simulator_serial_gps.h>
#include <test_device_serial.h>
#include <test_property_serial.h>
#include <test_fileutil.h>
#include <test_proc.h>
#include <test_proc_list.h>
#include <test_source_timer.h>
#include <test_source_gps_serial.h>
#include <test_source_gps_simulator.h>
#include <test_source_seatalk_simulator.h>
#include <test_source_seatalk_serial.h>
#include <test_destination_nmea_serial.h>
#include <test_destination_logbook.h>
#include <test_destination_dst_lua.h>
#include <test_destination_message_log.h>

#if defined(ENABLE_FILTER_LUA)
	#include <test_filter_lua.h>
#endif

#if defined(ENABLE_FILTER_LUA) || defined(ENABLE_DESTINATION_LUA) || defined(ENABLE_SOURCE_LUA)
	#include <test_lua_message.h>
#endif

#if defined(ENABLE_SOURCE_LUA)
	#include <test_source_src_lua.h>
#endif

int main()
{
	CU_initialize_registry();

	register_suite_strlist();
	register_suite_property();
	register_suite_nmea();
	register_suite_config();
	register_suite_filter_null();
	register_suite_filter_nmea();
	register_suite_filterlist();
	register_suite_device();
	register_suite_device_simulator_serial_gps();
	register_suite_device_serial();
	register_suite_property_serial();
	register_suite_fileutil();
	register_suite_proc();
	register_suite_proc_list();
	register_suite_source_timer();
	register_suite_source_gps_serial();
	register_suite_source_gps_simulator();
	register_suite_destination_nmea_serial();
	register_suite_destination_logbook();
	register_suite_destination_dst_lua();
	register_suite_destination_message_log();

#if defined(ENABLE_FILTER_LUA) || defined(ENABLE_DESTINATION_LUA) || defined(ENABLE_SOURCE_LUA)
	register_suite_lua_message();
#endif

#ifdef ENABLE_FILTER_LUA
	register_suite_filter_lua();
#endif

#if defined(ENABLE_SOURCE_LUA)
	register_suite_source_src_lua();
#endif

	register_suite_source_seatalk_simulator();
	register_suite_source_seatalk_serial();
	register_suite_seatalk();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	CU_cleanup_registry();
	return EXIT_SUCCESS;
}

