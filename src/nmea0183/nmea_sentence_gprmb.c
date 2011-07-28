#include <nmea0183/nmea_sentence_gprmb.h>
#include <nmea0183/nmea_util.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_rmb_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_RMB;
	v = &nmea->sentence.rmb;
	p = find_token_end(s);
	for (state = -1; state < 13 && s < e; ++state) {
		switch (state) {
			case  0: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			case  1: if (parse_fix(s, p, &v->cross_track_error) != p) return -1; break;
			case  2: v->steer_dir = (s == p) ? NMEA_LEFT : *s; break;
			case  3: if (parse_int(s, p, &v->waypoint_to) != p) return -1; break;
			case  4: if (parse_int(s, p, &v->waypoint_from) != p) return -1; break;
			case  5: if (parse_angle(s, p, &v->lat) != p && check_latitude(&v->lat)) return -1; break;
			case  6: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  7: if (parse_angle(s, p, &v->lon) != p && check_longitude(&v->lon)) return -1; break;
			case  8: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case  9: if (parse_fix(s, p, &v->range) != p) return -1; break;
			case 10: if (parse_fix(s, p, &v->bearing) != p) return -1; break;
			case 11: if (parse_fix(s, p, &v->dst_velocity) != p) return -1; break;
			case 12: v->arrival_status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_gprmb =
{
	.type = NMEA_RMB,
	.tag = NMEA_SENTENCE_GPRMB,
	.read= read,
	.write = NULL,
};

