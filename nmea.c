#include "nmea.h"
#include <string.h>
#include <ctype.h>

static uint8_t hex2i(char c)
{
	if ((c >= '0') && (c <= '9')) return c - '0';
	if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
	return 0xff;
}

/*
 * checks the checksum of the sentence.
 * @param[in] s sentence to check
 * @retval  0 success
 * @retval -1 failure
 */
static int checksum(const char * s)
{
	uint8_t chk = 0;
	if (!s || !(*s) || *s != '$') return -1;
	++s; /* skip '$' */
	for (; *s && *s != '*'; ++s) chk ^= *s;
	++s; /* skip '*' */
	return chk == hex2i(s[0]) * 16 + hex2i(s[1]) ? 0 : -1;
}

/*
 * returns a pointer to the position of the next comma within the
 * specified string.
 * @param[in] s the string to parse
 * @return the position of the next comma or, if none found, the end of the string
 */
static const char * find_comma(const char * s)
{
	while (s && *s && *s != ',')  ++s;
	return s;
}

/*
 * @param[in] v time to check
 * @retval 1 success
 * @retval 0 failure
 */
int check_time(const struct nmea_time_t * v)
{
	return 1
		&& v->h < 24
		&& v->m < 60
		&& v->s < 60
		&& v->ms < 1000
		;
}

/*
 * @param[in] v date to check
 * @retval 1 success
 * @retval 0 failure
 */
int check_date(const struct nmea_date_t * v)
{
	return 1
		&& v->m >= 1
		&& v->m <= 12
		&& v->d >= 1
		&& v->d <= 31
		;
}

/*
 * @param[in] v latitude to check
 * @retval 1 success
 * @retval 0 failure
 */
int check_latitude(const struct nmea_angle_t * v)
{
	return 1
		&& v->d < 90
		&& v->m < 60
		&& v->s < 600000
		;
}

/*
 * @param[in] v longitude to check
 * @retval 1 success
 * @retval 0 failure
 */
int check_longitude(const struct nmea_angle_t * v)
{
	return 1
		&& v->d < 180
		&& v->m < 60
		&& v->s < 600000
		;
}

/*
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_int(const char * s, const char * e, uint32_t * v)
{
	*v = 0;
	for (; *s && s < e; ++s) {
		if (!isdigit(*s)) return s;
		*v *= 10;
		*v += *s - '0';
	}
	return s;
}

/*
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_float(const char * s, const char * e, float * v)
{
	int state = 0;
	float f = 1.0f;
	*v = 0.0f;
	for (; *s && s < e; ++s) {
		switch (state) {
			case 0: /* before decimal point */
				if (*s == '.') {
					state = 1;
				} else if (isdigit(*s)) {
					*v *= 10.0f;
					*v += *s - '0';
				} else return s;
				break;
			case 1: /* after decimal point */
				if (!isdigit(*s)) return s;
				f *= 0.1;
				*v += f * (*s - '0');
				break;
		}
	}
	return s;
}

/* TODO */
const char * parse_fix(const char * s, const char * e, struct nmea_fix_t * v)
{
	int state = 0;
	v->i = 0;
	v->d = 0;
	for (; *s && s < e; ++s) {
		switch (state) {
			case 0: /* before decimal point */
				if (*s == '.') {
					state = 1;
				} else if (isdigit(*s)) {
					v->i *= 10;
					v->i += *s - '0';
				} else return s;
				break;
			case 1:
			/* TODO */
				break;
		}
	}
	return e;
}

/*
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_time(const char * s, const char * e, struct nmea_time_t * v)
{
	float t;
	unsigned int ft;
	const char * p;
	if (s == e) {
		v->h = 0;
		v->m = 0;
		v->s = 0;
		v->ms = 0;
		return e;
	}
	p = parse_float(s, e, &t);
	if (p == e) {
		ft = (unsigned int)t;
		v->h = (ft / 10000) % 100;
		v->m = (ft / 100) % 100;
		v->s = ft % 100;
		v->ms = (unsigned int)((t - ft) * 1000);
	}
	return p;
}

/*
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_date(const char * s, const char * e, struct nmea_date_t * v)
{
	uint32_t t;
	const char * p;
	if (s == e) {
		v->y = 0;
		v->m = 0;
		v->d = 0;
		return e;
	}
	p = parse_int(s, e, &t);
	if (p == e) {
		v->d = (t / 10000) % 100;
		v->m = (t / 100) % 100;
		v->y = t % 100;
	}
	return p;
}

/*
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_angle(const char * s, const char * e, struct nmea_angle_t * v)
{
	float t;
	unsigned int ft;
	const char * p;
	if (s == e) {
		v->d = 0;
		v->m = 0;
		v->s = 0;
		return e;
	}
	p = parse_float(s, e, &t);
	if (p == e) {
		ft = (unsigned int)t;
printf("%s:%d: t=%f  ft=%u\n", __FILE__, __LINE__, t, ft);
		v->d = ft / 100;
		v->m = ft % 100;
		v->s = (t-ft) * 60.0f;
	}
	return p;
}

/*
 * @param[in] s sentence to parse
 * @param[out] nmea parsed sentence
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gprmb(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_rmb_t * rmb = &nmea->sentence.rmb;
	while (*s && *p && *s != '*' && state >= 0) {
		p = find_comma(s);
		switch (state) {
			case  0: rmb->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			case  1: if (parse_float(s, p, &rmb->cross_track_error) != p) return -1; break;
			case  2: rmb->steer_dir = (s == p) ? NMEA_LEFT : *s; break;
			case  3: if (parse_int(s, p, &rmb->waypoint_to) != p) return -1; break;
			case  4: if (parse_int(s, p, &rmb->waypoint_from) != p) return -1; break;
			case  5: if (parse_angle(s, p, &rmb->lat) != p && !check_latitude(&rmb->lat)) return -1; break;
			case  6: rmb->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  7: if (parse_angle(s, p, &rmb->lon) != p && !check_longitude(&rmb->lon)) return -1; break;
			case  8: rmb->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case  9: if (parse_float(s, p, &rmb->range) != p) return -1; break;
			case 10: if (parse_float(s, p, &rmb->bearing) != p) return -1; break;
			case 11: if (parse_float(s, p, &rmb->dst_velocity) != p) return -1; break;
			case 12: rmb->arrival_status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			default: break;
		}
		s = p+1;
		++state;
	}
	return 0;
} /* }}} */

/*
 * @param[in] s sentence to parse
 * @param[out] nmea parsed sentence
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gprmc(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_rmc_t * rmc = &nmea->sentence.rmc;
	while (*s && *p && *s != '*' && state >= 0) {
		p = find_comma(s);
		switch (state) {
			case  0: if (parse_time(s, p, &rmc->time) != p && !check_time(&rmc->time)) return -1; break;
			case  1: rmc->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			case  2: if (parse_angle(s, p, &rmc->lat) != p && !check_latitude(&rmc->lat)) return -1; break;
			case  3: rmc->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  4: if (parse_angle(s, p, &rmc->lon) != p && !check_longitude(&rmc->lon)) return -1; break;
			case  5: rmc->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case  6: if (parse_float(s, p, &rmc->sog) != p) return -1; break;
			case  7: if (parse_float(s, p, &rmc->head) != p) return -1; break;
			case  8: if (parse_date(s, p, &rmc->date) != p || !check_date(&rmc->date)) return -1; break;
			case  9: if (parse_float(s, p, &rmc->m) != p) return -1; break;
			case 10: rmc->m_dir = (s == p) ? NMEA_EAST : *s; break;
			case 11: rmc->sig_integrity = (s == p) ? NMEA_SIG_INT_DATANOTVALID : *s; break;
			default: break;
		}
		s = p+1;
		++state;
	}
	return 0;
} /* }}} */

/*
 * @param[in] s sentence to parse
 * @param[out] nmea parsed sentence
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gpgga(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_gga_t * gga = &nmea->sentence.gga;
	while (*s && *p && *s != '*' && state >= 0) {
		p = find_comma(s);
		switch (state) {
			case  0: if (parse_time(s, p, &gga->time) != p && !check_time(&gga->time)) return -1; break;
			case  1: if (parse_angle(s, p, &gga->lat) != p && !check_latitude(&gga->lat)) return -1; break;
			case  2: gga->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  3: if (parse_angle(s, p, &gga->lon) != p && !check_longitude(&gga->lon)) return -1; break;
			case  4: gga->lon_dir = (s == p) ? NMEA_EAST : *s;  break;
			case  5: if (parse_int(s, p, &gga->quality) != p) return -1; break;
			case  6: if (parse_int(s, p, &gga->n_satelites) != p) return -1; break;
			case  7: if (parse_float(s, p, &gga->hor_dilution) != p) return -1; break;
			case  8: if (parse_float(s, p, &gga->height_antenna) != p) return -1; break;
			case  9: gga->unit_antenna = (s == p) ? NMEA_UNIT_METER : *s; break;
			case 10: if (parse_float(s, p, &gga->geodial_separation) != p) return -1; break;
			case 11: gga->unit_geodial_separation = (s == p) ? NMEA_UNIT_METER : *s; break;
			case 12: if (parse_float(s, p, &gga->dgps_age) != p) return -1; break;
			case 13: if (parse_int(s, p, &gga->dgps_ref) != p) return -1; break;
			default: break;
		}
		s = p+1;
		++state;
	};
	return 0;
} /* }}} */

/*
 * @param[in] s sentence to parse
 * @param[out] nmea parsed sentence
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gpgsv(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_gsv_t * gsv = &nmea->sentence.gsv;
	while (*s && *p && *s != '*' && state >= 0) {
		p = find_comma(s);
		switch (state) {
			case  0: if (parse_int(s, p, &gsv->n_messages) != p) return -1; break;
			case  1: if (parse_int(s, p, &gsv->message_number) != p) return -1; break;
			case  2: if (parse_int(s, p, &gsv->sat[0].id) != p) return -1; break;
			case  3: if (parse_int(s, p, &gsv->sat[0].elevation) != p) return -1; break;
			case  4: if (parse_int(s, p, &gsv->sat[0].azimuth) != p) return -1; break;
			case  5: if (parse_int(s, p, &gsv->sat[0].snr) != p) return -1; break;
			case  6: if (parse_int(s, p, &gsv->sat[1].id) != p) return -1; break;
			case  7: if (parse_int(s, p, &gsv->sat[1].elevation) != p) return -1; break;
			case  8: if (parse_int(s, p, &gsv->sat[1].azimuth) != p) return -1; break;
			case  9: if (parse_int(s, p, &gsv->sat[1].snr) != p) return -1; break;
			case 10: if (parse_int(s, p, &gsv->sat[2].id) != p) return -1; break;
			case 11: if (parse_int(s, p, &gsv->sat[2].elevation) != p) return -1; break;
			case 12: if (parse_int(s, p, &gsv->sat[2].azimuth) != p) return -1; break;
			case 13: if (parse_int(s, p, &gsv->sat[2].snr) != p) return -1; break;
			case 14: if (parse_int(s, p, &gsv->sat[3].id) != p) return -1; break;
			case 15: if (parse_int(s, p, &gsv->sat[3].elevation) != p) return -1; break;
			case 16: if (parse_int(s, p, &gsv->sat[3].azimuth) != p) return -1; break;
			case 17: if (parse_int(s, p, &gsv->sat[3].snr) != p) return -1; break;
			default: break;
		}
		s = p+1;
		++state;
	};
	return 0;
} /* }}} */

/*
 * @param[in] s sentence to parse
 * @param[out] nmea parsed sentence
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gpgsa(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_gsa_t * gsa = &nmea->sentence.gsa;
	while (*s && *p && *s != '*' && state >= 0) {
		p = find_comma(s);
		switch (state) {
			case  0: gsa->selection_mode = (s == p) ? '?' : *s; break; /* TODO */
			case  1: if (parse_int(s, p, &gsa->mode) != p) return -1; break;
			case  2: if (parse_int(s, p, &gsa->id[ 0]) != p) return -1; break;
			case  3: if (parse_int(s, p, &gsa->id[ 1]) != p) return -1; break;
			case  4: if (parse_int(s, p, &gsa->id[ 2]) != p) return -1; break;
			case  5: if (parse_int(s, p, &gsa->id[ 3]) != p) return -1; break;
			case  6: if (parse_int(s, p, &gsa->id[ 4]) != p) return -1; break;
			case  7: if (parse_int(s, p, &gsa->id[ 5]) != p) return -1; break;
			case  8: if (parse_int(s, p, &gsa->id[ 6]) != p) return -1; break;
			case  9: if (parse_int(s, p, &gsa->id[ 7]) != p) return -1; break;
			case 10: if (parse_int(s, p, &gsa->id[ 8]) != p) return -1; break;
			case 11: if (parse_int(s, p, &gsa->id[ 9]) != p) return -1; break;
			case 12: if (parse_int(s, p, &gsa->id[10]) != p) return -1; break;
			case 13: if (parse_int(s, p, &gsa->id[11]) != p) return -1; break;
			case 14: if (parse_float(s, p, &gsa->pdop) != p) return -1; break;
			case 15: if (parse_float(s, p, &gsa->hdop) != p) return -1; break;
			case 16: if (parse_float(s, p, &gsa->vdop) != p) return -1; break;
			default: break;
		}
		s = p+1;
		++state;
	};
	return 0;
} /* }}} */

/*
 * @param[in] s sentence to parse
 * @param[out] nmea parsed sentence
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gpgll(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_gll_t * gll = &nmea->sentence.gll;
	while (*s && *p && *s != '*' && state >= 0) {
		p = find_comma(s);
		switch (state) {
			case 0: if (parse_angle(s, p, &gll->lat) != p && !check_latitude(&gll->lat)) return -1; break;
			case 1: gll->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case 2: if (parse_angle(s, p, &gll->lon) != p && !check_longitude(&gll->lon)) return -1; break;
			case 3: gll->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case 4: if (parse_time(s, p, &gll->time) != p && !check_time(&gll->time)) return -1; break;
			case 5: gll->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			default: break;
		}
		s = p+1;
		++state;
	}
	return 0;
} /* }}} */

/*
 * @param[in]  s read sentence
 * @param[out] nmea data of the parsed structure
 * @retval  0 success
 * @retval  1 unknown sentence
 * @retval -1 parameter error
 * @retval -2 checksum error
 */
int nmea_read(const char * s, struct nmea_t * nmea) /* {{{ */
{
	struct entry_t {
		const char * tag;
		int (*parser)(const char *, struct nmea_t *);
		uint32_t type;
	};

	static const struct entry_t TAB[] = {
		{ "GPRMB", parse_gprmb, NMEA_RMB },
		{ "GPRMC", parse_gprmc, NMEA_RMC },
		{ "GPGGA", parse_gpgga, NMEA_GGA },
		{ "GPGSV", parse_gpgsv, NMEA_GSV },
		{ "GPGSA", parse_gpgsa, NMEA_GSA },
		{ "GPGLL", parse_gpgll, NMEA_GLL },
		{ NULL,    NULL,        0        }
	};

	const char * p = s;
	const struct entry_t * entry = NULL;

	if (!s || !nmea) return -1;
	if (checksum(s)) return -2;
	if (*s != '$') return -1;
	memset(nmea, 0, sizeof(*nmea));
	++s;
	p = find_comma(s);
	for (entry = TAB; entry && entry->tag; ++entry) {
		if (!strncmp(s, entry->tag, p-s)) {
			nmea->type = entry->type;
			return entry->parser(p+1, nmea) ? -1 : 0;
		}
	}
	return 1;
} /* }}} */

