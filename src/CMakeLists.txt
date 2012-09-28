
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

add_subdirectory(lua)
add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(device)
add_subdirectory(nmea)
add_subdirectory(navcom)
add_subdirectory(test)

add_executable(navd
	navd.c
	)

target_link_libraries(navd
	navcom
	devices
	nmea
	config
	common
	lua
	m
	)

