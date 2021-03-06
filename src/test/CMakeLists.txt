
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

add_executable(serial_hexdump
	serial_hexdump.c
	)

if (NEEDS_NMEA)
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
endif()

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
	test_config.c
	test_filter_null.c
	test_filterlist.c
	test_device.c
	test_device_simulator_serial_gps.c
	test_device_serial.c
	test_property_serial.c
	test_fileutil.c
	test_proc.c
	test_proc_list.c
	test_source_timer.c
	test_destination_message_log.c
	)

set(LIBRARIES
	navcom
	config
	devices
	)

if (ENABLE_SOURCE_GPSSERIAL)
	set(TEST_SOURCES ${TEST_SOURCES} test_source_gps_serial.c)
endif()

if (ENABLE_SOURCE_GPSSIMULATOR)
	set(TEST_SOURCES ${TEST_SOURCES} test_source_gps_simulator.c)
endif()

if (ENABLE_SOURCE_SEATALKSERIAL)
	set(TEST_SOURCES ${TEST_SOURCES} test_source_seatalk_serial.c)
endif()

if (ENABLE_SOURCE_SEATALKSIMULATOR)
	set(TEST_SOURCES ${TEST_SOURCES} test_source_seatalk_simulator.c)
endif()

if (ENABLE_SOURCE_LUA)
	set(TEST_SOURCES ${TEST_SOURCES} test_source_src_lua.c)
endif()

if (ENABLE_DESTINATION_LUA)
	set(TEST_SOURCES ${TEST_SOURCES} test_destination_dst_lua.c)
endif()

if (ENABLE_DESTINATION_LOGBOOK)
	set(TEST_SOURCES ${TEST_SOURCES} test_destination_logbook.c)
endif()

if (ENABLE_DESTINATION_NMEASERIAL)
	set(TEST_SOURCES ${TEST_SOURCES} test_destination_nmea_serial.c)
endif()

if (ENABLE_FILTER_LUA)
	set(TEST_SOURCES ${TEST_SOURCES} test_filter_lua.c)
endif()

if (ENABLE_FILTER_NMEA)
	set(TEST_SOURCES ${TEST_SOURCES} test_filter_nmea.c)
endif()

if (ENABLE_FILTER_SEATALK_TO_NMEA)
	set(TEST_SOURCES ${TEST_SOURCES} test_filter_seatalk_to_nmea.c)
endif()

if (NEEDS_NMEA)
	set(TEST_SOURCES ${TEST_SOURCES} test_nmea.c)
	set(LIBRARIES ${LIBRARIES} nmea)
endif()

if (NEEDS_SEATALK)
	set(TEST_SOURCES ${TEST_SOURCES} test_seatalk.c)
	set(LIBRARIES ${LIBRARIES} seatalk)
endif()

if (NEEDS_LUA)
	set(TEST_SOURCES ${TEST_SOURCES} test_lua_message.c)

	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../lua/include
		)

	set(LIBRARIES ${LIBRARIES} lua)
endif()

add_executable(testrunner
	testrunner.c
	${TEST_SOURCES}
	)

target_link_libraries(testrunner
	cunit
	${LIBRARIES}
	common
	m
	)

