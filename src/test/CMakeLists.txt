
add_subdirectory(cunit)

add_executable(dump
	dump.c
	)

target_link_libraries(dump
	devices
	nmea
	common
	)

add_executable(check_size
	check_size.c
	)

target_link_libraries(check_size
	devices
	nmea
	common
	)

add_executable(config_test
	config_test.c
	)

target_link_libraries(config_test
	config
	common
	)

add_executable(config_test_1
	config_test_1.c
	)

target_link_libraries(config_test_1
	config
	common
	)

include_directories(
	"cunit/include"
	"."
	)

set(TEST_SOURCES
	test_strlist.c
	test_property.c
	test_nmea.c
	test_config.c
	test_filter_null.c
	test_filter_nmea.c
	test_filterlist.c
	test_device.c
	test_property_serial.c
	test_fileutil.c
	)

if (ENABLE_FILTER_LUA)
	set(TEST_SOURCES ${TEST_SOURCES} test_filter_lua.c)
endif()

add_executable(testrunner
	testrunner.c
	${TEST_SOURCES}
	)

target_link_libraries(testrunner
	cunit
	navcom
	config
	nmea
	devices
	lua
	common
	m
	)

