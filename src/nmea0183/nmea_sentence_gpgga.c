#include <nmea0183/nmea_sentence_gpgga.h>
#include <nmea0183/nmea_util.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_gga_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GGA;
	v = &nmea->sentence.gga;
	p = find_token_end(s);
	for (state = -1; state < 14 && s < e; ++state) {
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
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgga =
{
	.type = NMEA_GGA,
	.tag = NMEA_SENTENCE_GPGGA,
	.read = read,
	.write = NULL,
};

