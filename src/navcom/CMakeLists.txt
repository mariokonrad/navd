
add_library(navcom
	proc.c
	proc_list.c
	filter_list.c
	property_serial.c
	source/gps_simulator.c
	source/gps_serial.c
	source/timer.c
	destination/message_log.c
	destination/nmea_serial.c
	destination/logbook.c
	filter/filter_null.c
	filter/filter_nmea.c
	)

