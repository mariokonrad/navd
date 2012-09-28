
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
add_executable(testrunner
	testrunner.c
	test_strlist.c
	test_property.c
	test_nmea.c
	test_config.c
	test_filter_null.c
	test_filter_nmea.c
	test_filter_lua.c
	)
target_link_libraries(testrunner
	cunit
	navcom
	config
	nmea
	lua
	common
	m
	)

