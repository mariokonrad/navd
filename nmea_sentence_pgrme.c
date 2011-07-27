#include <nmea_sentence_pgrme.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_pgrme(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_garmin_rme_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GARMIN_RME;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.garmin_rme;
	switch (state) {
		case 0: if (parse_fix(s, p, &v->hpe) != p) return -1; break;
		case 1: v->unit_hpe = (s == p) ? NMEA_UNIT_METER : *s; break;
		case 2: if (parse_fix(s, p, &v->vpe) != p) return -1; break;
		case 3: v->unit_vpe = (s == p) ? NMEA_UNIT_METER : *s; break;
		case 4: if (parse_fix(s, p, &v->sepe) != p) return -1; break;
		case 5: v->unit_sepe= (s == p) ? NMEA_UNIT_METER : *s; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_pgrme =
{
	.type = NMEA_GARMIN_RME,
	.tag = NMEA_SENTENCE_PGRME,
	.parse = sentence_parser_pgrme,
	.write = NULL,
};

