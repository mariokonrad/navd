#include <nmea_sentence_gpgga.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_gpgga(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_gga_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GGA;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.gga;
	switch (state) {
		case  0: if (parse_time(s, p, &v->time) != p && check_time(&v->time)) return -1; break;
		case  1: if (parse_angle(s, p, &v->lat) != p && check_latitude(&v->lat)) return -1; break;
		case  2: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
		case  3: if (parse_angle(s, p, &v->lon) != p && check_longitude(&v->lon)) return -1; break;
		case  4: v->lon_dir = (s == p) ? NMEA_EAST : *s;  break;
		case  5: if (parse_int(s, p, &v->quality) != p) return -1; break;
		case  6: if (parse_int(s, p, &v->n_satelites) != p) return -1; break;
		case  7: if (parse_fix(s, p, &v->hor_dilution) != p) return -1; break;
		case  8: if (parse_fix(s, p, &v->height_antenna) != p) return -1; break;
		case  9: v->unit_antenna = (s == p) ? NMEA_UNIT_METER : *s; break;
		case 10: if (parse_fix(s, p, &v->geodial_separation) != p) return -1; break;
		case 11: v->unit_geodial_separation = (s == p) ? NMEA_UNIT_METER : *s; break;
		case 12: if (parse_fix(s, p, &v->dgps_age) != p) return -1; break;
		case 13: if (parse_int(s, p, &v->dgps_ref) != p) return -1; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgga =
{
	.type = NMEA_GGA,
	.tag = NMEA_SENTENCE_GPGGA,
	.parse = sentence_parser_gpgga,
	.write = NULL,
};

