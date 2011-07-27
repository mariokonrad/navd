#include <nmea_sentence_gpvtg.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_gpvtg(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_vtg_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_BOD;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.vtg;
	switch (state) {
		case  0: if (parse_fix(s, p, &v->track_true) != p) return -1; break;
		case  1: v->type_true = (s == p) ? NMEA_TRUE : *s; break;
		case  2: if (parse_fix(s, p, &v->track_magn) != p) return -1; break;
		case  3: v->type_magn = (s == p) ? NMEA_MAGNETIC : *s; break;
		case  4: if (parse_fix(s, p, &v->speed_kn) != p) return -1; break;
		case  5: v->unit_speed_kn = (s == p) ? NMEA_UNIT_KNOT : *s; break;
		case  6: if (parse_fix(s, p, &v->speed_kmh) != p) return -1; break;
		case  7: v->unit_speed_kmh = (s == p) ? NMEA_UNIT_KMH : *s; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpvtg =
{
	.type = NMEA_VTG,
	.tag = NMEA_SENTENCE_GPVTG,
	.parse = sentence_parser_gpvtg,
	.write = NULL,
};

