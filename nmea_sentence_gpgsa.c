#include <nmea_sentence_gpgsa.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_gpgsa(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_gsa_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GSA;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.gsa;
	switch (state) {
		case  0: v->selection_mode = (s == p) ? NMEA_SELECTIONMODE_AUTOMATIC : *s; break;
		case  1: if (parse_int(s, p, &v->mode) != p) return -1; break;
		case  2:
		case  3:
		case  4:
		case  5:
		case  6:
		case  7:
		case  8:
		case  9:
		case 10:
		case 11:
		case 12:
		case 13: if (parse_int(s, p, &v->id[state-2]) != p) return -1; break;
		case 14: if (parse_fix(s, p, &v->pdop) != p) return -1; break;
		case 15: if (parse_fix(s, p, &v->hdop) != p) return -1; break;
		case 16: if (parse_fix(s, p, &v->vdop) != p) return -1; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgsa =
{
	.type = NMEA_GSA,
	.tag = NMEA_SENTENCE_GPGSA,
	.parse = sentence_parser_gpgsa,
	.write = NULL,
};

