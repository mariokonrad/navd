
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(common)
add_subdirectory(device)
add_subdirectory(nmea)
add_subdirectory(test)

add_executable(navd
	navd.c
	)

target_link_libraries(navd
	devices
	nmea
	common
	pthread
	)
