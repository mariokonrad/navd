# common

set(COMMON
	""
	)

if (ENABLE_FILTER_LUA OR ENABLE_DESTINATION_LUA)
	set(COMMON ${COMMON} common/lua_helper.c)
	set(COMMON ${COMMON} common/lua_syslog.c)
	set(COMMON ${COMMON} common/lua_debug.c)
	set(COMMON ${COMMON} common/lua_nmea.c)
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/../lua/include
		)
endif()

# sources

set(SOURCES
	source/timer.c
	)

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

add_library(navcom
	proc.c
	proc_list.c
	filter_list.c
	property_serial.c
	property_read.c
	${COMMON}
	${SOURCES}
	${DESTINATIONS}
	${FILTERS}
	)

