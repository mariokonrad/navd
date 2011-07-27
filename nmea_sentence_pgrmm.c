#include <nmea_sentence_pgrmm.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_pgrmm(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_garmin_rmm_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GARMIN_RMM;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.garmin_rmm;
	switch (state) {
		case 0: if ((unsigned int)(p-s+1) < sizeof(v->map_datum) && parse_str(s, p, v->map_datum) != p) return -1; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_pgrmm =
{
	.type = NMEA_GARMIN_RMM,
	.tag = NMEA_SENTENCE_PGRMM,
	.parse = sentence_parser_pgrmm,
	.write = NULL,
};

