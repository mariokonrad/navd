
add_subdirectory(cunit)

add_executable(dump
	dump.c
	)

target_link_libraries(dump
	devices
	nmea
	common
	)

add_executable(nmea_test
	nmea_test.c
	)

target_link_libraries(nmea_test
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

add_executable(strlist_test
	strlist_test.c
	)

target_link_libraries(strlist_test
	common
	)

include_directories("cunit/include")
add_executable(testrunner
	testrunner.c
	)
target_link_libraries(testrunner
	cunit
	)

