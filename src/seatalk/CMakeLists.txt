
file(GLOB SENTENCES seatalk_sentence_*.c)

add_library(seatalk
	seatalk.c
	seatalk_base.c
	seatalk_util.c
	${SENTENCES}
	)

