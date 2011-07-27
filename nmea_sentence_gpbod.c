#include <nmea_sentence_gpbod.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_gpbod(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_bod_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_BOD;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.bod;
	switch (state) {
		case  0: if (parse_fix(s, p, &v->bearing_true) != p) return -1; break;
		case  1: v->type_true = (s == p) ? NMEA_TRUE : *s; break;
		case  2: if (parse_fix(s, p, &v->bearing_magn) != p) return -1; break;
		case  3: v->type_magn = (s == p) ? NMEA_MAGNETIC : *s; break;
		case  4: if (parse_int(s, p, &v->waypoint_to) != p) return -1; break;
		case  5: if (parse_int(s, p, &v->waypoint_from) != p) return -1; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpbod =
{
	.type = NMEA_BOD,
	.tag = NMEA_SENTENCE_GPBOD,
	.parse = sentence_parser_gpbod,
	.write = NULL,
};

