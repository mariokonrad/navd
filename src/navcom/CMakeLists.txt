# sources

set(SOURCES
	source/timer.c
	)

if (ENABLE_SOURCE_LUA)
	set(SOURCES ${SOURCES} source/src_lua.c)
endif()

if (ENABLE_SOURCE_GPSSERIAL)
	set(SOURCES ${SOURCES} source/gps_serial.c)
endif()

if (ENABLE_SOURCE_GPSSIMULATOR)
	set(SOURCES ${SOURCES} source/gps_simulator.c)
endif()

# destinations

set(DESTINATIONS
	destination/message_log.c
	destination/nmea_serial.c
	destination/logbook.c
	)

if (ENABLE_DESTINATION_LUA)
	set(DESTINATIONS ${DESTINATIONS} destination/dst_lua.c)
endif()

# filters

set(FILTERS
	filter/filter_null.c
	filter/filter_nmea.c
	)

if (ENABLE_FILTER_LUA)
	set(FILTERS ${FILTERS} filter/filter_lua.c)
endif()

# common

set(COMMON
	proc.c
	proc_list.c
	filter_list.c
	property_serial.c
	property_read.c
	)

if (ENABLE_FILTER_LUA OR ENABLE_DESTINATION_LUA OR ENABLE_SOURCE_LUA)
	set(COMMON ${COMMON}
		lua_helper.c
		lua_syslog.c
		lua_debug.c
		lua_nmea.c
		lua_message.c
		)
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../lua/include
		)
endif()

add_library(navcom
	${COMMON}
	${SOURCES}
	${DESTINATIONS}
	${FILTERS}
	)

