
if (CMAKE_COMPILER_IS_GNUCC)
	if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -m64")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ggdb -m64")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG -m64")
	elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra -m32")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ggdb -m32")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG -m32")
	else()
		# pedantic and c99 disabled because of sigaction/etc and pselect
		set(CMAKE_C_FLAGS         "-O2 -Wall -Wextra")
		set(CMAKE_C_FLAGS_DEBUG   "-O0 -Wall -Wextra -ggdb")
		set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Wextra -DNDEBUG")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS         "")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "-fprofile-arcs -ftest-coverage")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
endif()

add_subdirectory(cunit)

add_executable(dump
	dump.c
	)

target_link_libraries(dump
	devices
	nmea
	common
	m
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
	test_device_simulator.c
	test_device_serial.c
	test_property_serial.c
	test_fileutil.c
	test_proc.c
	test_proc_list.c
	test_source_timer.c
	test_source_gps_serial.c
	test_source_gps_simulator.c
	test_destination_nmea_serial.c
	test_destination_logbook.c
	test_destination_dst_lua.c
	test_destination_message_log.c
	)

if (ENABLE_FILTER_LUA OR ENABLE_DESTINATION_LUA OR ENABLE_SOURCE_LUA)
	set(TEST_SOURCES ${TEST_SOURCES} test_lua_message.c)
	set(TEST_SOURCES ${TEST_SOURCES} test_filter_lua.c)
	set(TEST_SOURCES ${TEST_SOURCES} test_source_src_lua.c)

	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../lua/include
		)
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

