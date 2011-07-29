#include <nmea/nmea_sentence_gpgsa.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_int.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_gsa_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GSA;
	v = &nmea->sentence.gsa;
	p = find_token_end(s);
	for (state = -1; state < 17 && s < e; ++state) {
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
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgsa =
{
	.type = NMEA_GSA,
	.tag = "GPGSA",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

