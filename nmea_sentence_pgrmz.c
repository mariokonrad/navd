#include <nmea_sentence_pgrmz.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_pgrmz(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_garmin_rmz_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GARMIN_RMZ;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.garmin_rmz;
	switch (state) {
		case 0: if (parse_fix(s, p, &v->alt) != p) return -1; break;
		case 1: v->unit_alt = (s == p) ? NMEA_UNIT_FEET : *s; break;
		case 2: if (parse_int(s, p, &v->pos_fix_dim) != p) return -1; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_pgrmz =
{
	.type = NMEA_GARMIN_RMZ,
	.tag = NMEA_SENTENCE_PGRMZ,
	.parse = sentence_parser_pgrmz,
	.write = NULL,
};

