#include <nmea/nmea_sentence_gprmc.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_checksum.h>
#include <stdio.h>

#define TAG "GPRMC"

/**
 * Reads the NMEA sentence into the specified structure.
 *
 * @param[out] nmea Structure to hold the parsed data.
 * @param[in] s Start of the string to parse (inclusive).
 * @param[in] e End of the string to parse (exclusive).
 * @retval -1 Parameter failure, parsing error.
 * @retval  0 Success
 */
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
			case  0: if (nmea_time_parse(s, p, &v->time) != p && nmea_time_check(&v->time)) return -1; break;
			case  1: v->status = (s == p) ? NMEA_STATUS_WARNING : *s; break;
			case  2: if (nmea_angle_parse(s, p, &v->lat) != p && nmea_check_latitude(&v->lat)) return -1; break;
			case  3: v->lat_dir = (s == p) ? NMEA_NORTH : *s; break;
			case  4: if (nmea_angle_parse(s, p, &v->lon) != p && nmea_check_longitude(&v->lon)) return -1; break;
			case  5: v->lon_dir = (s == p) ? NMEA_EAST : *s; break;
			case  6: if (nmea_fix_parse(s, p, &v->sog) != p) return -1; break;
			case  7: if (nmea_fix_parse(s, p, &v->head) != p) return -1; break;
			case  8: if (nmea_date_parse(s, p, &v->date) != p && nmea_date_check(&v->date)) return -1; break;
			case  9: if (nmea_fix_parse(s, p, &v->m) != p) return -1; break;
			case 10: v->m_dir = (s == p) ? NMEA_EAST : *s; break;
			case 11: v->sig_integrity = (s == p) ? NMEA_SIG_INT_DATANOTVALID : *s; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

/**
 * Writes the NMEA sentence defined by the structure to the specified buffer.
 *
 * @param[out] buf The buffer to contain the resulting NMEA sentence.
 * @param[in] size The size of the buffer.
 * @param[in] nmea The NMEA data to write to the buffer.
 * @retval -1 Parameter failure.
 * @return Number of characters written to the buffer.
 */
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
			case  1: rc = write_string(p, r, TAG); break;
			case  2: rc = write_char(p, r, ','); break;
			case  3: if (nmea_time_check_zero(&v->time)) rc = nmea_time_write(p, r, &v->time); break;
			case  4: rc = write_char(p, r, ','); break;
			case  5: rc = write_char(p, r, v->status); break;
			case  6: rc = write_char(p, r, ','); break;
			case  7: if (nmea_angle_check_zero(&v->lat)) rc = nmea_write_latitude(p, r, &v->lat); break;
			case  8: rc = write_char(p, r, ','); break;
			case  9: if (nmea_angle_check_zero(&v->lat)) rc = write_char(p, r, v->lat_dir); break;
			case 10: rc = write_char(p, r, ','); break;
			case 11: if (nmea_angle_check_zero(&v->lon)) rc = nmea_write_lonitude(p, r, &v->lon); break;
			case 12: rc = write_char(p, r, ','); break;
			case 13: if (nmea_angle_check_zero(&v->lon)) rc = write_char(p, r, v->lon_dir); break;
			case 14: rc = write_char(p, r, ','); break;
			case 15: if (nmea_fix_check_zero(&v->sog)) rc = nmea_fix_write(p, r, &v->sog, 1, 1); break;
			case 16: rc = write_char(p, r, ','); break;
			case 17: if (nmea_fix_check_zero(&v->head)) rc = nmea_fix_write(p, r, &v->head, 1, 1); break;
			case 18: rc = write_char(p, r, ','); break;
			case 19: if (nmea_date_check_zero(&v->date)) rc = nmea_date_write(p, r, &v->date); break;
			case 20: rc = write_char(p, r, ','); break;
			case 21: if (nmea_fix_check_zero(&v->m)) rc = nmea_fix_write(p, r, &v->m, 1, 1); break;
			case 22: rc = write_char(p, r, ','); break;
			case 23: if (nmea_fix_check_zero(&v->m)) rc = write_char(p, r, v->m_dir); break;
			case 24: rc = write_char(p, r, ','); break;
			case 25: rc = write_char(p, r, v->sig_integrity); chksum_end = p + 1; break;
			case 26: rc = write_char(p, r, '*'); break;
			case 27: rc = nmea_checksum_write(p, r, chksum_start, chksum_end); break;
			default: rc = -1; break;
		}
	}

	return (int)i;
}

/**
 * Byte order conversion of the data from host to network byte order.
 */
static void hton(struct nmea_t * nmea)
{
	struct nmea_rmc_t * v;

	if (nmea == NULL) return;
	v = &nmea->sentence.rmc;
	nmea_time_hton(&v->time);
	nmea_angle_hton(&v->lat);
	nmea_angle_hton(&v->lon);
	nmea_fix_hton(&v->sog);
	nmea_fix_hton(&v->head);
	nmea_date_hton(&v->date);
	nmea_fix_hton(&v->m);
}

/**
 * Byte order conversion of the data from network to host byte order.
 */
static void ntoh(struct nmea_t * nmea)
{
	struct nmea_rmc_t * v;

	if (nmea == NULL) return;
	v = &nmea->sentence.rmc;
	nmea_time_ntoh(&v->time);
	nmea_angle_ntoh(&v->lat);
	nmea_angle_ntoh(&v->lon);
	nmea_fix_ntoh(&v->sog);
	nmea_fix_ntoh(&v->head);
	nmea_date_ntoh(&v->date);
	nmea_fix_ntoh(&v->m);
}

/**
 * Description of the NMEA sentence.
 */
const struct nmea_sentence_t sentence_gprmc =
{
	.type = NMEA_RMC,
	.tag = TAG,
	.read = read,
	.write = write,
	.hton = hton,
	.ntoh = ntoh,
};

