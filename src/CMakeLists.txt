
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/global_config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/global_config.h
	)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(device)
add_subdirectory(navcom)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	add_subdirectory(test)
endif()

add_executable(navd
	navd.c
	daemon.c
	programoptions.c
	registry.c
	route.c
	)

set(LIBRARIES
	navcom
	devices
	config
	)

if (NEEDS_NMEA)
	add_subdirectory(nmea)
	set(LIBRARIES ${LIBRARIES} nmea)
endif()

if (NEEDS_SEATALK)
	add_subdirectory(seatalk)
	set(LIBRARIES ${LIBRARIES} seatalk)
endif()

if (NEEDS_LUA)
	add_subdirectory(lua)
	set(LIBRARIES ${LIBRARIES} lua)
endif()

target_link_libraries(navd
	${LIBRARIES}
	common
	m
	)

install(TARGETS navd
	RUNTIME
	DESTINATION bin
	COMPONENT applications
	)


