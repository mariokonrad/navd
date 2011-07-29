
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(common)
add_subdirectory(device)
add_subdirectory(nmea)

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

