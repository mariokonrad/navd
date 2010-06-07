#include "nmea.h"
#include <string.h>
#include <ctype.h>

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
		&& v->s.i < 60
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
		&& v->s.i < 60
		;
}

/*
 * copies the string between [s,e) into v, the user has to make
 * sure there is enough room within v.
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_str(const char * s, const char * e, char * v)
{
	if (!v) return e;
	if (s != e) {
		memcpy(v, s, e-s);
	}
	return e;
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
const char * parse_fix(const char * s, const char * e, struct nmea_fix_t * v)
{
	uint32_t f = NMEA_FIX_DECIMALS;
	int state = 0;
	v->i = 0;
	v->d = 0;
	for (; *s && s < e && f > 0; ++s) {
		switch (state) {
			case 0:
				if (*s == '.') {
					state = 1;
				} else if (isdigit(*s)) {
					v->i *= 10;
					v->i += *s - '0';
				} else return s;
				break;
			case 1:
				if (!isdigit(*s)) return s;
				f /= 10;
				v->d += f * (*s - '0');
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
	struct nmea_fix_t t;
	const char * p;
	if (s == e) {
		v->h = 0;
		v->m = 0;
		v->s = 0;
		v->ms = 0;
		return e;
	}
	p = parse_fix(s, e, &t);
	if (p == e) {
		v->h = (t.i / 10000) % 100;
		v->m = (t.i / 100) % 100;
		v->s = t.i % 100;
		v->ms = t.d / 1000;
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
	struct nmea_fix_t t;
	const char * p;
	if (s == e) {
		v->d = 0;
		v->m = 0;
		v->s.i = 0;
		v->s.d = 0;
		return e;
	}
	p = parse_fix(s, e, &t);
	if (p == e) {
		v->d = t.i / 100;
		v->m = t.i % 100;
		v->s.i = (t.d * 60) / NMEA_FIX_DECIMALS;
		v->s.d = (t.d * 60) % NMEA_FIX_DECIMALS;
	}
	return p;
}

static int sentence_gprmb(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_rmb_t * v = &nmea->sentence.rmb;
	switch (state) {
		case -1: nmea->type = NMEA_RMB; break;
		case  0: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
		case  1: if (parse_fix(s, p, &v->cross_track_error) != p) return -1; break;
		case  2: v->steer_dir = (s == p) ? NMEA_LEFT : *s; break;
		case  3: if (parse_int(s, p, &v->waypoint_to) != p) return -1; break;
		case  4: if (parse_int(s, p, &v->waypoint_from) != p) return -1; break;
		case  5: if (parse_angle(s, p, &v->lat) != p && !check_latitude(&v->lat)) return -1; break;
		case  6: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
		case  7: if (parse_angle(s, p, &v->lon) != p && !check_longitude(&v->lon)) return -1; break;
		case  8: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
		case  9: if (parse_fix(s, p, &v->range) != p) return -1; break;
		case 10: if (parse_fix(s, p, &v->bearing) != p) return -1; break;
		case 11: if (parse_fix(s, p, &v->dst_velocity) != p) return -1; break;
		case 12: v->arrival_status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gprmc(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_rmc_t * v = &nmea->sentence.rmc;
	switch (state) {
		case -1: nmea->type = NMEA_RMC; break;
		case  0: if (parse_time(s, p, &v->time) != p && !check_time(&v->time)) return -1; break;
		case  1: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
		case  2: if (parse_angle(s, p, &v->lat) != p && !check_latitude(&v->lat)) return -1; break;
		case  3: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
		case  4: if (parse_angle(s, p, &v->lon) != p && !check_longitude(&v->lon)) return -1; break;
		case  5: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
		case  6: if (parse_fix(s, p, &v->sog) != p) return -1; break;
		case  7: if (parse_fix(s, p, &v->head) != p) return -1; break;
		case  8: if (parse_date(s, p, &v->date) != p && !check_date(&v->date)) return -1; break;
		case  9: if (parse_fix(s, p, &v->m) != p) return -1; break;
		case 10: v->m_dir = (s == p) ? NMEA_EAST : *s; break;
		case 11: v->sig_integrity = (s == p) ? NMEA_SIG_INT_DATANOTVALID : *s; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gpgga(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_gga_t * v = &nmea->sentence.gga;
	switch (state) {
		case -1: nmea->type = NMEA_GGA; break;
		case  0: if (parse_time(s, p, &v->time) != p && !check_time(&v->time)) return -1; break;
		case  1: if (parse_angle(s, p, &v->lat) != p && !check_latitude(&v->lat)) return -1; break;
		case  2: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
		case  3: if (parse_angle(s, p, &v->lon) != p && !check_longitude(&v->lon)) return -1; break;
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
} /* }}} */

static int sentence_gpgsv(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_gsv_t * v = &nmea->sentence.gsv;
	switch (state) {
		case -1: nmea->type = NMEA_GSV; break;
		case  0: if (parse_int(s, p, &v->n_messages) != p) return -1; break;
		case  1: if (parse_int(s, p, &v->message_number) != p) return -1; break;
		case  2: if (parse_int(s, p, &v->sat[0].id) != p) return -1; break;
		case  3: if (parse_int(s, p, &v->sat[0].elevation) != p) return -1; break;
		case  4: if (parse_int(s, p, &v->sat[0].azimuth) != p) return -1; break;
		case  5: if (parse_int(s, p, &v->sat[0].snr) != p) return -1; break;
		case  6: if (parse_int(s, p, &v->sat[1].id) != p) return -1; break;
		case  7: if (parse_int(s, p, &v->sat[1].elevation) != p) return -1; break;
		case  8: if (parse_int(s, p, &v->sat[1].azimuth) != p) return -1; break;
		case  9: if (parse_int(s, p, &v->sat[1].snr) != p) return -1; break;
		case 10: if (parse_int(s, p, &v->sat[2].id) != p) return -1; break;
		case 11: if (parse_int(s, p, &v->sat[2].elevation) != p) return -1; break;
		case 12: if (parse_int(s, p, &v->sat[2].azimuth) != p) return -1; break;
		case 13: if (parse_int(s, p, &v->sat[2].snr) != p) return -1; break;
		case 14: if (parse_int(s, p, &v->sat[3].id) != p) return -1; break;
		case 15: if (parse_int(s, p, &v->sat[3].elevation) != p) return -1; break;
		case 16: if (parse_int(s, p, &v->sat[3].azimuth) != p) return -1; break;
		case 17: if (parse_int(s, p, &v->sat[3].snr) != p) return -1; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gpgsa(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_gsa_t * v = &nmea->sentence.gsa;
	switch (state) {
		case -1: nmea->type = NMEA_GSA; break;
		case  0: v->selection_mode = (s == p) ? NMEA_SELECTIONMODE_AUTOMATIC : *s; break;
		case  1: if (parse_int(s, p, &v->mode) != p) return -1; break;
		case  2:
		case  3:
		case  4:
		case  5:
		case  6:
		case  7:
		case  8:
		case  9:
		case 10:
		case 11:
		case 12:
		case 13: if (parse_int(s, p, &v->id[state-2]) != p) return -1; break;
		case 14: if (parse_fix(s, p, &v->pdop) != p) return -1; break;
		case 15: if (parse_fix(s, p, &v->hdop) != p) return -1; break;
		case 16: if (parse_fix(s, p, &v->vdop) != p) return -1; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gpgll(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_gll_t * v = &nmea->sentence.gll;
	switch (state) {
		case -1: nmea->type = NMEA_GLL; break;
		case  0: if (parse_angle(s, p, &v->lat) != p && !check_latitude(&v->lat)) return -1; break;
		case  1: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
		case  2: if (parse_angle(s, p, &v->lon) != p && !check_longitude(&v->lon)) return -1; break;
		case  3: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
		case  4: if (parse_time(s, p, &v->time) != p && !check_time(&v->time)) return -1; break;
		case  5: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gpbod(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_bod_t * v = &nmea->sentence.bod;
	switch (state) {
		case -1: nmea->type = NMEA_BOD; break;
		case  0: if (parse_fix(s, p, &v->bearing_true) != p) return -1; break;
		case  1: v->type_true = (s == p) ? NMEA_TRUE : *s; break;
		case  2: if (parse_fix(s, p, &v->bearing_magn) != p) return -1; break;
		case  3: v->type_magn = (s == p) ? NMEA_MAGNETIC : *s; break;
		case  4: if (parse_int(s, p, &v->waypoint_to) != p) return -1; break;
		case  5: if (parse_int(s, p, &v->waypoint_from) != p) return -1; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gpvtg(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_vtg_t * v = &nmea->sentence.vtg;
	switch (state) {
		case -1: nmea->type = NMEA_BOD; break;
		case  0: if (parse_fix(s, p, &v->track_true) != p) return -1; break;
		case  1: v->type_true = (s == p) ? NMEA_TRUE : *s; break;
		case  2: if (parse_fix(s, p, &v->track_magn) != p) return -1; break;
		case  3: v->type_magn = (s == p) ? NMEA_MAGNETIC : *s; break;
		case  4: if (parse_fix(s, p, &v->speed_kn) != p) return -1; break;
		case  5: v->unit_speed_kn = (s == p) ? NMEA_UNIT_KNOT : *s; break;
		case  6: if (parse_fix(s, p, &v->speed_kmh) != p) return -1; break;
		case  7: v->unit_speed_kmh = (s == p) ? NMEA_UNIT_KMH : *s; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_gprte(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_rte_t * v = &nmea->sentence.rte;
	switch (state) {
		case -1: nmea->type = NMEA_RTE; break;
		case  0: if (parse_int(s, p, &v->n_messages) != p) return -1; break;
		case  1: if (parse_int(s, p, &v->message_number) != p) return -1; break;
		case  2: v->message_mode = (s == p) ? NMEA_COMPLETE_ROUTE : *s; break;
		case  3:
		case  4:
		case  5:
		case  6:
		case  7:
		case  8:
		case  9:
		case 10:
		case 11:
		case 12: if ((unsigned int)(p-s+1) < sizeof(v->waypoint_id[state-3]) && parse_str(s, p, v->waypoint_id[state-3]) != p) return -1; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_pgrme(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_garmin_rme_t * v = &nmea->sentence.garmin_rme;
	switch (state) {
		case -1: nmea->type = NMEA_GARMIN_RME; break;
		case 0: if (parse_fix(s, p, &v->hpe) != p) return -1; break;
		case 1: v->unit_hpe = (s == p) ? NMEA_UNIT_METER : *s; break;
		case 2: if (parse_fix(s, p, &v->vpe) != p) return -1; break;
		case 3: v->unit_vpe = (s == p) ? NMEA_UNIT_METER : *s; break;
		case 4: if (parse_fix(s, p, &v->sepe) != p) return -1; break;
		case 5: v->unit_sepe= (s == p) ? NMEA_UNIT_METER : *s; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_pgrmz(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_garmin_rmz_t * v = &nmea->sentence.garmin_rmz;
	switch (state) {
		case -1: nmea->type = NMEA_GARMIN_RMZ; break;
		case 0: if (parse_fix(s, p, &v->alt) != p) return -1; break;
		case 1: v->unit_alt = (s == p) ? NMEA_UNIT_FEET : *s; break;
		case 2: if (parse_int(s, p, &v->pos_fix_dim) != p) return -1; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_pgrmm(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_garmin_rmm_t * v = &nmea->sentence.garmin_rmm;
	switch (state) {
		case -1: nmea->type = NMEA_GARMIN_RMM; break;
		case 0: if ((unsigned int)(p-s+1) < sizeof(v->map_datum) && parse_str(s, p, v->map_datum) != p) return -1; break;
		default: break;
	}
	return 0;
} /* }}} */

static int sentence_hchdg(int state, const char * s, const char * p, struct nmea_t * nmea) /* {{{ */
{
	struct nmea_hc_hdg_t * v = &nmea->sentence.hc_hdg;
	switch (state) {
		case -1: nmea->type = NMEA_HC_HDG; break;
		case 0: if (parse_fix(s, p, &v->heading) != p) return -1; break;
		case 1: if (parse_fix(s, p, &v->magn_dev) != p) return -1; break;
		case 2: v->magn_dev_dir = (s == p) ? NMEA_EAST : *s; break;
		case 3: if (parse_fix(s, p, &v->magn_var) != p) return -1; break;
		case 4: v->magn_dev_dir = (s == p) ? NMEA_EAST : *s; break;
		default: break;
	}
	return 0;
} /* }}} */

/*
 * returns a pointer to the position of the next comma within the
 * specified string.
 * @param[in] s the string to parse
 * @return the position of the next comma or, if none found, the end of the string
 */
static const char * find_token_end(const char * s)
{
	while (s && *s && *s != ',' && *s != '*')  ++s;
	return s;
}

/*
 * @retval 1 token seems to be valid
 * @retval 0 token seems to be invalid
 */
static int token_valid(const char * s, const char * p)
{
	return *s && *p && *s != '*' && *p != '*';
}

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
 * parses the sentence using the specified token parser.
 *
 * @param[out] nmea the structure to be filled
 * @param[in] s the sentence to parse
 * @param[in] parser the token parser
 */
static int parse_sentence(struct nmea_t * nmea, const char * s, int (*parse)(int, const char *, const char *, struct nmea_t *)) /* {{{ */
{
	int state = 0;
	const char * p = s;
	parse(-1, NULL, NULL, nmea);
	for (; token_valid(s, p); s = p+1, ++state) {
		p = find_token_end(s);
		if (parse(state, s, p, nmea)) return -1;
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
		int (*parser)(int, const char *, const char *, struct nmea_t *);
	};

	static const struct entry_t TAB[] = {
		{ "GPRMB", sentence_gprmb },
		{ "GPRMC", sentence_gprmc },
		{ "GPGGA", sentence_gpgga },
		{ "GPGSV", sentence_gpgsv },
		{ "GPGSA", sentence_gpgsa },
		{ "GPGLL", sentence_gpgll },
		{ "GPBOD", sentence_gpbod },
		{ "GPVTG", sentence_gpvtg },
		{ "GPRTE", sentence_gprte },
		{ "PGRME", sentence_pgrme },
		{ "PGRMM", sentence_pgrmm },
		{ "PGRMZ", sentence_pgrmz },
		{ "HCHDG", sentence_hchdg },
		{ NULL,    NULL           }
	};

	const char * p = s;
	const struct entry_t * entry = NULL;

	if (!s || !nmea) return -1;
	if (checksum(s)) return -2;
	if (*s != '$') return -1;
	memset(nmea, 0, sizeof(*nmea));
	++s;
	p = find_token_end(s);
	for (entry = TAB; entry && entry->tag; ++entry) {
		if (!strncmp(s, entry->tag, p-s)) {
			return parse_sentence(nmea, p+1, entry->parser) ? -1 : 0;
		}
	}
	return 1;
} /* }}} */

