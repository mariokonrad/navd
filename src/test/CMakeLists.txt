
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

add_executable(check_size
	check_size.c
	)

target_link_libraries(check_size
	devices
	nmea
	common
	)

add_executable(config_test
	config_test.c
	)

target_link_libraries(config_test
	config
	common
	)

