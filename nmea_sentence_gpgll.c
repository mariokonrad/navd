#include <nmea_sentence_gpgll.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_gpgll(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_gll_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GLL;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.gll;
	switch (state) {
		case  0: if (parse_angle(s, p, &v->lat) != p && check_latitude(&v->lat)) return -1; break;
		case  1: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
		case  2: if (parse_angle(s, p, &v->lon) != p && check_longitude(&v->lon)) return -1; break;
		case  3: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
		case  4: if (parse_time(s, p, &v->time) != p && check_time(&v->time)) return -1; break;
		case  5: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgll =
{
	.type = NMEA_GLL,
	.tag = NMEA_SENTENCE_GPGLL,
	.parse = sentence_parser_gpgll,
	.write = NULL,
};

