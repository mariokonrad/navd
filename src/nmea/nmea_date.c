#include <nmea/nmea_date.h>
#include <nmea/nmea_int.h>
#include <common/endian.h>
#include <stdio.h>

/**
 * Checks whether all members of the date information are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
int nmea_date_check_zero(const struct nmea_date_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->y == 0)
		&& (v->m == 0)
		&& (v->d == 0)
		) ? 0 : -2;
}

/**
 * Checks whether the specified date is valid.
 *
 * @param[in] v date to check
 * @retval 0 success
 * @retval -1 Parameter error
 * @retval -2 Date is invalid
 */
int nmea_date_check(const struct nmea_date_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->m >= 1)
		&& (v->m <= 12)
		&& (v->d >= 1)
		&& (v->d <= 31)
		) ? 0 : -2;
}

/**
 * Parses the date within the specified string (start and end of string).
 * If the string is empty, the result will be initialized to zero.
 *
 * The date to be parsed must be in the form: "DDMMYY"
 *  - DD : zero leading day of the month (01..31)
 *  - MM : zero leading month of the year (01..12)
 *  - YY : two digit year, 00 = 2000
 *
 * @note This function does not do any range checks. To perform checks,
 *       use nmea_date_check()
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * nmea_date_parse(const char * s, const char * e, struct nmea_date_t * v)
{
	uint32_t t;
	const char * p;
	if (s == NULL || e == NULL || v == NULL) return NULL;
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

/**
 * Writers date information in the format DDMMYY to the specified buffer.
 * This format is required by the NMEA data representation.
 * The date is written only if it passes the nmea_date_check() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The date information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Date data is wrong.
 */
int nmea_date_write(char * buf, uint32_t size, const struct nmea_date_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 6) return -1;
	if (nmea_date_check(v)) return -2;
	return snprintf(buf, size, "%02u%02u%02u", v->d, v->m, v->y % 100);
}

/**
 * Converts the byte order of the specified date from host to network order.
 *
 * @param[inout] v Date to convert.
 */
void nmea_date_hton(struct nmea_date_t * v)
{
	if (v == NULL) return;
	v->y = endian_hton_32(v->y);
	v->m = endian_hton_32(v->m);
	v->d = endian_hton_32(v->d);
}

/**
 * Converts the byte order of the specified date from network to host order.
 *
 * @param[inout] v Date to convert.
 */
void nmea_date_ntoh(struct nmea_date_t * v)
{
	if (v == NULL) return;
	v->y = endian_ntoh_32(v->y);
	v->m = endian_ntoh_32(v->m);
	v->d = endian_ntoh_32(v->d);
}

