#include <nmea/nmea_time.h>
#include <nmea/nmea_fix.h>
#include <common/endian.h>
#include <stdio.h>

/* Checks whether all members of the time information are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
int nmea_time_check_zero(const struct nmea_time_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->h == 0)
		&& (v->m == 0)
		&& (v->s == 0)
		&& (v->ms == 0)
		) ? 0 : -2;
}

/* Checks whether the specified time is valid.
 *
 * @param[in] v time to check
 * @retval 0 success
 * @retval -1 Parameter error
 * @retval -2 Time is invalid
 */
int nmea_time_check(const struct nmea_time_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->h < 24)
		&& (v->m < 60)
		&& (v->s < 60)
		&& (v->ms < 1000)
		) ? 0 : -2
		;
}

/* TODO
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * nmea_time_parse(const char * s, const char * e, struct nmea_time_t * v)
{
	struct nmea_fix_t t;
	const char * p;
	if (s == NULL || e == NULL || v == NULL) return NULL;
	if (s == e) {
		v->h = 0;
		v->m = 0;
		v->s = 0;
		v->ms = 0;
		return e;
	}
	p = nmea_fix_parse(s, e, &t);
	if (p == e) {
		v->h = (t.i / 10000) % 100;
		v->m = (t.i / 100) % 100;
		v->s = t.i % 100;
		v->ms = t.d / 1000;
	}
	return p;
}

/* Writers time information in the format HHMMSS to the specified buffer.
 * This format is required by the NMEA data representation.
 * The time is written only if it passes the nmea_time_check() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The time information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Time data is wrong.
 */
int nmea_time_write(char * buf, uint32_t size, const struct nmea_time_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 6) return -1;
	if (nmea_time_check(v)) return -2;
	return snprintf(buf, size, "%02u%02u%02u", v->h, v->m, v->s);
}

void nmea_time_hton(struct nmea_time_t * v)
{
	if (v == NULL) return;
	v->h = endian_hton_32(v->h);
	v->m = endian_hton_32(v->m);
	v->s = endian_hton_32(v->s);
	v->ms = endian_hton_32(v->ms);
}

void nmea_time_ntoh(struct nmea_time_t * v)
{
	if (v == NULL) return;
	v->h = endian_ntoh_32(v->h);
	v->m = endian_ntoh_32(v->m);
	v->s = endian_ntoh_32(v->s);
	v->ms = endian_ntoh_32(v->ms);
}

