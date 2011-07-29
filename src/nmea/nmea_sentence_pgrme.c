#include <nmea/nmea_sentence_pgrme.h>
#include <nmea/nmea_util.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_garmin_rme_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GARMIN_RME;
	v = &nmea->sentence.garmin_rme;
	p = find_token_end(s);
	for (state = -1; state < 6 && s < e; ++state) {
		switch (state) {
			case 0: if (parse_fix(s, p, &v->hpe) != p) return -1; break;
			case 1: v->unit_hpe = (s == p) ? NMEA_UNIT_METER : *s; break;
			case 2: if (parse_fix(s, p, &v->vpe) != p) return -1; break;
			case 3: v->unit_vpe = (s == p) ? NMEA_UNIT_METER : *s; break;
			case 4: if (parse_fix(s, p, &v->sepe) != p) return -1; break;
			case 5: v->unit_sepe= (s == p) ? NMEA_UNIT_METER : *s; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_pgrme =
{
	.type = NMEA_GARMIN_RME,
	.tag = "PGRME",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

