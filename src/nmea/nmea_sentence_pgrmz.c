#include <nmea/nmea_sentence_pgrmz.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_int.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_garmin_rmz_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GARMIN_RMZ;
	v = &nmea->sentence.garmin_rmz;
	p = find_token_end(s);
	for (state = -1; state < 3 && s < e; ++state) {
		switch (state) {
			case 0: if (nmea_fix_parse(s, p, &v->alt) != p) return -1; break;
			case 1: v->unit_alt = (s == p) ? NMEA_UNIT_FEET : *s; break;
			case 2: if (parse_int(s, p, &v->pos_fix_dim) != p) return -1; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_pgrmz =
{
	.type = NMEA_GARMIN_RMZ,
	.tag = "PGRMZ",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

