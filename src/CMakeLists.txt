cmake_minimum_required(VERSION 2.6)

file(GLOB NMEA0183_SRC nmea0183/*.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(dump
	dump.c
	${NMEA0183_SRC}
	)

add_executable(nmea_test
	nmea_test.c
	${NMEA0183_SRC}
	)

