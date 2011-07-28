#include <nmea/nmea_sentence_pgrmm.h>
#include <nmea/nmea_util.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_garmin_rmm_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GARMIN_RMM;
	v = &nmea->sentence.garmin_rmm;
	p = find_token_end(s);
	for (state = -1; state < 1 && s < e; ++state) {
		switch (state) {
			case 0: if ((unsigned int)(p-s+1) < sizeof(v->map_datum) && parse_str(s, p, v->map_datum) != p) return -1; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_pgrmm =
{
	.type = NMEA_GARMIN_RMM,
	.tag = NMEA_SENTENCE_PGRMM,
	.read = read,
	.write = NULL,
};

