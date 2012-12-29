
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
add_subdirectory(nmea)
add_subdirectory(navcom)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	add_subdirectory(test)
endif()

add_executable(navd
	navd.c
	)

set(LIBRARIES
	navcom
	devices
	nmea
	config
	common
	)

if (ENABLE_FILTER_LUA)
	add_subdirectory(lua)
	set(LIBRARIES ${LIBRARIES} lua)
endif()

target_link_libraries(navd
	${LIBRARIES}
	m
	)

