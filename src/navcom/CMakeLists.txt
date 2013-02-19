
set(SOURCES
	source/gps_serial.c
	source/timer.c
	)

if (ENABLE_SOURCE_GPSSIMULATOR)
	set(SOURCES ${SOURCES} source/gps_simulator.c)
endif()

set(DESTINATIONS
	destination/message_log.c
	destination/nmea_serial.c
	destination/logbook.c
	)

set(FILTERS
	filter/filter_null.c
	filter/filter_nmea.c
	)

if (ENABLE_FILTER_LUA)
	set(FILTERS ${FILTERS} filter/filter_lua.c)

	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../lua/include
		)
endif()

add_library(navcom
	proc.c
	proc_list.c
	filter_list.c
	property_serial.c
	property_read.c
	${SOURCES}
	${DESTINATIONS}
	${FILTERS}
	)

