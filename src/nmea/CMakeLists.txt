
file(GLOB SENTENCES nmea_sentence_*.c)

add_library(nmea
	nmea.c
	nmea_base.c
	nmea_util.c
	${SENTENCES}
	)

