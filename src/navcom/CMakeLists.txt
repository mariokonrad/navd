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

if (ENABLE_SOURCE_SEATALKSERIAL)
	set(SOURCES ${SOURCES} source/seatalk_serial.c)
endif()

if (ENABLE_SOURCE_GPSSIMULATOR)
	set(SOURCES ${SOURCES} source/gps_simulator.c)
endif()

if (ENABLE_SOURCE_SEATALKSIMULATOR)
	set(SOURCES ${SOURCES} source/seatalk_simulator.c)
endif()

# destinations

set(DESTINATIONS
	destination/message_log.c
	)

if (ENABLE_DESTINATION_LUA)
	set(DESTINATIONS ${DESTINATIONS} destination/dst_lua.c)
endif()

if (ENABLE_DESTINATION_LOGBOOK)
	set(DESTINATIONS ${DESTINATIONS} destination/logbook.c)
endif()

if (ENABLE_DESTINATION_NMEASERIAL)
	set(DESTINATIONS ${DESTINATIONS} destination/nmea_serial.c)
endif()

# filters

set(FILTERS
	filter/filter_null.c
	)

if (ENABLE_FILTER_LUA)
	set(FILTERS ${FILTERS} filter/filter_lua.c)
endif()

if (ENABLE_FILTER_NMEA)
	set(FILTERS ${FILTERS} filter/filter_nmea.c)
endif()

# common

set(COMMON
	proc.c
	proc_list.c
	filter_list.c
	property_serial.c
	property_read.c
	message_comm.c
	)

if (NEEDS_LUA)
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

