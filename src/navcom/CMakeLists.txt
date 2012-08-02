
add_library(navcom
	proc.c
	proc_list.c
	filter_list.c
	destination/message_log.c
	source/gps_simulator.c
	source/gps_serial.c
	source/timer.c
	filter/filter_null.c
	filter/filter_nmea.c
	)

