
file(GLOB SENTENCES nmea_sentence_*.c)

add_library(nmea
	nmea.c
	nmea_base.c
	nmea_util.c
	nmea_fix.c
	nmea_int.c
	nmea_time.c
	nmea_date.c
	nmea_angle.c
	nmea_checksum.c
	${SENTENCES}
	)

