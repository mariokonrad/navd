#include <nmea_sentence_gprmc.h>
#include <nmea_util.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_rmc_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_RMC;
	v = &nmea->sentence.rmc;
	p = find_token_end(s);
	for (state = -1; state < 12 && s < e; ++state) {
		switch (state) {
			case  0: if (parse_time(s, p, &v->time) != p && check_time(&v->time)) return -1; break;
			case  1: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			case  2: if (parse_angle(s, p, &v->lat) != p && check_latitude(&v->lat)) return -1; break;
			case  3: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  4: if (parse_angle(s, p, &v->lon) != p && check_longitude(&v->lon)) return -1; break;
			case  5: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case  6: if (parse_fix(s, p, &v->sog) != p) return -1; break;
			case  7: if (parse_fix(s, p, &v->head) != p) return -1; break;
			case  8: if (parse_date(s, p, &v->date) != p && check_date(&v->date)) return -1; break;
			case  9: if (parse_fix(s, p, &v->m) != p) return -1; break;
			case 10: v->m_dir = (s == p) ? NMEA_EAST : *s; break;
			case 11: v->sig_integrity = (s == p) ? NMEA_SIG_INT_DATANOTVALID : *s; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

static int write(char * buf, uint32_t size, const struct nmea_t * nmea)
{
	const struct nmea_rmc_t * v;
	uint32_t i = 0;
	int rc = 0;
	int state;
	char * p;
	uint32_t r;
	const char * chksum_start = NULL;
	const char * chksum_end = NULL;

	if (buf == NULL || size == 0 || nmea == NULL) return -1;
	if (nmea->type != NMEA_RMC) return -1;
	v = &nmea->sentence.rmc;
	p = buf;
	r = size;

	for (state = 0; rc >= 0; ++state) {
		i += rc;
		p += rc;
		r -= rc;
		rc = 0;
		switch (state) {
			case  0: rc = write_char(p, r, START_TOKEN_NMEA); chksum_start = p + 1; break;
			case  1: rc = write_string(p, r, NMEA_SENTENCE_GPRMC); break;
			case  2: rc = write_char(p, r, ','); break;
			case  3: if (check_time_zero(&v->time)) rc = write_time(p, r, &v->time); break;
			case  4: rc = write_char(p, r, ','); break;
			case  5: rc = write_char(p, r, v->status); break;
			case  6: rc = write_char(p, r, ','); break;
			case  7: if (check_angle_zero(&v->lat)) rc = write_lat(p, r, &v->lat); break;
			case  8: rc = write_char(p, r, ','); break;
			case  9: if (check_angle_zero(&v->lat)) rc = write_char(p, r, v->lat_dir); break;
			case 10: rc = write_char(p, r, ','); break;
			case 11: if (check_angle_zero(&v->lon)) rc = write_lon(p, r, &v->lon); break;
			case 12: rc = write_char(p, r, ','); break;
			case 13: if (check_angle_zero(&v->lon)) rc = write_char(p, r, v->lon_dir); break;
			case 14: rc = write_char(p, r, ','); break;
			case 15: if (check_fix_zero(&v->sog)) rc = write_fix(p, r, &v->sog, 1, 1); break;
			case 16: rc = write_char(p, r, ','); break;
			case 17: if (check_fix_zero(&v->head)) rc = write_fix(p, r, &v->head, 1, 1); break;
			case 18: rc = write_char(p, r, ','); break;
			case 19: if (check_date_zero(&v->date)) rc = write_date(p, r, &v->date); break;
			case 20: rc = write_char(p, r, ','); break;
			case 21: if (check_fix_zero(&v->m)) rc = write_fix(p, r, &v->m, 1, 1); break;
			case 22: rc = write_char(p, r, ','); break;
			case 23: if (check_fix_zero(&v->m)) rc = write_char(p, r, v->m_dir); break;
			case 24: rc = write_char(p, r, ','); break;
			case 25: rc = write_char(p, r, v->sig_integrity); chksum_end = p + 1; break;
			case 26: rc = write_char(p, r, '*'); break;
			case 27: rc = write_checksum(p, r, chksum_start, chksum_end); break;
			default: rc = -1; break;
		}
	}

	return (int)i;
}

const struct nmea_sentence_t sentence_gprmc =
{
	.type = NMEA_RMC,
	.tag = NMEA_SENTENCE_GPRMC,
	.read = read,
	.write = write,
};

