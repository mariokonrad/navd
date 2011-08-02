#include <nmea/nmea_sentence_gpgll.h>
#include <nmea/nmea_util.h>
#include <stdio.h>

/**
 * @todo Documenation
 */
static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_gll_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GLL;
	v = &nmea->sentence.gll;
	p = find_token_end(s);
	for (state = -1; state < 6 && s < e; ++state) {
		switch (state) {
			case  0: if (nmea_angle_parse(s, p, &v->lat) != p && nmea_check_latitude(&v->lat)) return -1; break;
			case  1: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  2: if (nmea_angle_parse(s, p, &v->lon) != p && nmea_check_longitude(&v->lon)) return -1; break;
			case  3: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case  4: if (nmea_time_parse(s, p, &v->time) != p && nmea_time_check(&v->time)) return -1; break;
			case  5: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

/**
 * @todo Documenation
 */
const struct nmea_sentence_t sentence_gpgll =
{
	.type = NMEA_GLL,
	.tag = "GPGLL",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

