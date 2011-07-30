#include <nmea/nmea_angle.h>
#include <common/endian.h>
#include <stdio.h>

/* Checks whether all members of the angle information are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
/* TODO:TEST */
int nmea_angle_check_zero(const struct nmea_angle_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->d == 0)
		&& (v->m == 0)
		&& (v->s.i == 0)
		&& (v->s.d == 0)
		) ? 0 : -2;
}

/* Checks whether the specified latitude is valid.
 *
 * @param[in] v latitude to check
 * @retval 0 success
 * @retval -1 Parameter error
 * @retval -2 Latitude is invalid
 */
int nmea_check_latitude(const struct nmea_angle_t * v)
{
	if (v == NULL) return -1;
	return (0
		|| ((v->d < 90) && (v->m < 60) && (v->s.i < 60) && (v->s.d < NMEA_FIX_DECIMALS))
		|| ((v->d == 90) && (v->m == 0) && (v->s.i == 0) && (v->s.d == 0))
		) ? 0 : -2;
}

/* Checks whether the specified longitude is valid.
 *
 * @param[in] v longitude to check
 * @retval 0 success
 * @retval -1 Parameter error
 * @retval -2 Longitude is invalid
 */
int nmea_check_longitude(const struct nmea_angle_t * v)
{
	if (v == NULL) return -1;
	return (0
		|| ((v->d < 180) && (v->m < 60) && (v->s.i < 60) && (v->s.d < NMEA_FIX_DECIMALS))
		|| ((v->d == 180) && (v->m == 0) && (v->s.i == 0) && (v->s.d == 0))
		) ? 0 : -2
		;
}

/* Parses an angle from a string between s and e into the specified structure.
 * This function handles angles in with the following EBNF:
 *
 *  angle   := degrees minutes '.' dec-min
 *  degrees := ['0'..'9']{1,3}
 *  minutes := ['0'..'9']{2}
 *  dec-min := ['0'..'9']{0,4}
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * nmea_angle_parse(const char * s, const char * e, struct nmea_angle_t * v)
{
	struct nmea_fix_t t;
	const char * p;
	if (s == NULL || e == NULL || v == NULL) return NULL;
	if (s == e) {
		v->d = 0;
		v->m = 0;
		v->s.i = 0;
		v->s.d = 0;
		return e;
	}
	p = nmea_fix_parse(s, e, &t);
	if (p == e) {
		v->d = t.i / 100;
		v->m = t.i % 100;
		v->s.i = (t.d * 60) / NMEA_FIX_DECIMALS;
		v->s.d = (t.d * 60) % NMEA_FIX_DECIMALS;
	}
	return p;
}

/* Writers latitude information in the format DDMM.SSSS to the specified buffer.
 * This format is required by the NMEA data representation.
 * The latitude is written only if it passes the nmea_check_latitude() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The latitude information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Latitude data is wrong.
 */
int nmea_write_latitude(char * buf, uint32_t size, const struct nmea_angle_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 9) return -1;
	if (nmea_check_latitude(v)) return -1;

	/* division by 100 is to achieve 4 decimal digits, maybe it would be better to use write_fix() */
	return snprintf(buf, size, "%02u%02u.%04u", v->d, v->m, (v->s.i * NMEA_FIX_DECIMALS + v->s.d) / 60 / 100);
}

/* Writers longitude information in the format DDDMM.SSSS to the specified buffer.
 * This format is required by the NMEA data representation.
 * The longitude is written only if it passes the nmea_check_longitude() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The longitude information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Longitude data is wrong.
 */
int nmea_write_lonitude(char * buf, uint32_t size, const struct nmea_angle_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 10) return -1;
	if (nmea_check_longitude(v)) return -1;

	/* division by 100 is to achieve 4 decimal digits, maybe it would be better to use write_fix() */
	return snprintf(buf, size, "%03u%02u.%04u", v->d, v->m, (v->s.i * NMEA_FIX_DECIMALS + v->s.d) / 60 / 100);
}

void nmea_angle_hton(struct nmea_angle_t * v)
{
	if (v == NULL) return;
	v->d = endian_hton_32(v->d);
	v->m = endian_hton_32(v->m);
	nmea_fix_hton(&v->s);
}

void nmea_angle_ntoh(struct nmea_angle_t * v)
{
	if (v == NULL) return;
	v->d = endian_ntoh_32(v->d);
	v->m = endian_ntoh_32(v->m);
	nmea_fix_ntoh(&v->s);
}

