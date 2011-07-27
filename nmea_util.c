#include "nmea_util.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Calculates and returns the checksum between the two position of a string.
 * The checksum is calculated inclusive start token and exclusive the end
 * token. If either one is wrong 0x00 will return.
 */
uint8_t checksum(const char * s, const char * e)
{
	uint8_t chk = 0x00;
	for (; *s && *e && s != e; ++s) chk ^= *s;
	return chk;
}

/* Converts the specified hexadecimal character to its numerical representation.
 * If the character does not match a hexadecimal digit, 0xff will return.
 */
static uint8_t hex2i(char c)
{
	if ((c >= '0') && (c <= '9')) return c - '0';
	if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
	return 0xff;
}

/* Returns the hexadecimal digit for the specified value.
 */
static char i2hex(int i)
{
	static const char * TAB = "0123456789ABCDEF";
	return (i >= 0 && i <= 15) ? TAB[i] : '\0';
}

/* TODO: test
 * Checks the checksum of the sentence.
 *
 * @param[in] s sentence to check
 * @retval  0 success
 * @retval -1 failure
 */
int check_checksum(const char * s, char start_token)
{
	/* TODO: use function checksum() */
	uint8_t chk = 0;
	if (!s || !(*s) || *s != start_token) return -1;
	++s; /* skip start token */
	for (; *s && *s != '*'; ++s) chk ^= *s;
	++s; /* skip '*' */
	return chk == hex2i(s[0]) * 16 + hex2i(s[1]) ? 0 : -1;
}

/* Checks whether all members of the fix number are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
/* TODO:TEST */
int check_fix_zero(const struct nmea_fix_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->i == 0)
		&& (v->d == 0)
		) ? 0 : -2;
}

/* Checks whether all members of the time information are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
/* TODO:TEST */
int check_time_zero(const struct nmea_time_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->h == 0)
		&& (v->m == 0)
		&& (v->s == 0)
		&& (v->ms == 0)
		) ? 0 : -2;
}

/* TODO
 *
 * @param[in] v time to check
 * @retval 0 success
 * @retval -1 failure
 */
int check_time(const struct nmea_time_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->h < 24)
		&& (v->m < 60)
		&& (v->s < 60)
		&& (v->ms < 1000)
		) ? 0 : -1
		;
}

/* Checks whether all members of the date information are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
/* TODO:TEST */
int check_date_zero(const struct nmea_date_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->y == 0)
		&& (v->m == 0)
		&& (v->d == 0)
		) ? 0 : -2;
}

/* TODO
 *
 * @param[in] v date to check
 * @retval 0 success
 * @retval -1 failure
 */
int check_date(const struct nmea_date_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->m >= 1)
		&& (v->m <= 12)
		&& (v->d >= 1)
		&& (v->d <= 31)
		) ? 0 : -1;
}

/* Checks whether all members of the angle information are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
/* TODO:TEST */
int check_angle_zero(const struct nmea_angle_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->d == 0)
		&& (v->m == 0)
		&& (v->s.i == 0)
		&& (v->s.d == 0)
		) ? 0 : -2;
}

/* TODO
 *
 * @param[in] v latitude to check
 * @retval 0 success
 * @retval -1 failure
 */
int check_latitude(const struct nmea_angle_t * v)
{
	if (v == NULL) return -1;
	return (0
		|| ((v->d < 90) && (v->m < 60) && (v->s.i < 60) && (v->s.d < NMEA_FIX_DECIMALS))
		|| ((v->d == 90) && (v->m == 0) && (v->s.i == 0) && (v->s.d == 0))
		) ? 0 : -1;
}

/* TODO
 *
 * @param[in] v longitude to check
 * @retval 0 success
 * @retval -1 failure
 */
int check_longitude(const struct nmea_angle_t * v)
{
	if (v == NULL) return -1;
	return (0
		|| ((v->d < 180) && (v->m < 60) && (v->s.i < 60) && (v->s.d < NMEA_FIX_DECIMALS))
		|| ((v->d == 180) && (v->m == 0) && (v->s.i == 0) && (v->s.d == 0))
		) ? 0 : -1
		;
}

/* Copies the string between [s,e) into v, the user has to make
 * sure there is enough room within v.
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_str(const char * s, const char * e, char * v)
{
	if (v == NULL) return e;
	if (s < e) {
		memcpy(v, s, e-s);
	}
	return e;
}

/* TODO
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_int(const char * s, const char * e, uint32_t * v)
{
	if (s == NULL || e == NULL || v == NULL) return NULL;
	*v = 0;
	for (; *s && s < e; ++s) {
		if (!isdigit((int)*s)) return s;
		*v *= 10;
		*v += *s - '0';
	}
	return s;
}

/* Parses a fix point number between start and end of the string into the
 * specified structure. This function handles the following EBNF:
 *
 *  fix := ['0'..'9']+ { '.' ['0' .. '9']{0,6} }
 *
 * Leading zeroes do not cause the number to be interpreted as octal,
 * the number must always be in decimal.
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_fix(const char * s, const char * e, struct nmea_fix_t * v)
{
	uint32_t f = NMEA_FIX_DECIMALS;
	int state = 0;
	if (s == NULL || e == NULL || v == NULL) return NULL;
	v->i = 0;
	v->d = 0;
	for (; *s && s < e && f > 0; ++s) {
		switch (state) {
			case 0:
				if (*s == '.') {
					state = 1;
				} else if (isdigit((int)*s)) {
					v->i *= 10;
					v->i += *s - '0';
				} else return s;
				break;
			case 1:
				if (!isdigit((int)*s)) return s;
				f /= 10;
				v->d += f * (*s - '0');
				break;
		}
	}
	return e;
}

/* TODO
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_time(const char * s, const char * e, struct nmea_time_t * v)
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
	p = parse_fix(s, e, &t);
	if (p == e) {
		v->h = (t.i / 10000) % 100;
		v->m = (t.i / 100) % 100;
		v->s = t.i % 100;
		v->ms = t.d / 1000;
	}
	return p;
}

/* TODO
 *
 * @param[in] s start of string to parse (inclusive)
 * @param[in] e end of string to parse (exclusive)
 * @param[out] v parsed value
 * @return position of the last valid character
 */
const char * parse_date(const char * s, const char * e, struct nmea_date_t * v)
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
const char * parse_angle(const char * s, const char * e, struct nmea_angle_t * v)
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
	p = parse_fix(s, e, &t);
	if (p == e) {
		v->d = t.i / 100;
		v->m = t.i % 100;
		v->s.i = (t.d * 60) / NMEA_FIX_DECIMALS;
		v->s.d = (t.d * 60) % NMEA_FIX_DECIMALS;
	}
	return p;
}

/* Writes a string into the buffer if there is enough room within the buffer.
 *
 * @param[out] buf The buffer to hold the data.
 * @param[in] size Number of bytes free in buffer.
 * @param[in] s String to write into the buffer.
 * @retval >= 0 Number of bytes written into the buffer.
 * @retval -1 Parameter error
 * @retval -2 Buffer too small
 */
int write_string(char * buf, uint32_t size, const char * s)
{
	uint32_t len;

	if (buf == NULL || size == 0 || s == NULL) return -1;
	len = strlen(s);
	if (len > size) return -2;
	strncat(buf, s, len);
	return (int)len;
}

/* Writes a character into the buffer if there is enough room within the buffer.
 *
 * @param[out] buf The buffer to hold the data.
 * @param[in] size Number of bytes free in buffer.
 * @param[in] c Character to write.
 * @retval >= 0 Number of bytes written into the buffer.
 * @retval -1 Parameter error
 */
int write_char(char * buf, uint32_t size, const char c)
{
	if (buf == NULL || size == 0) return -1;
	*buf++ = c;
	*buf = '\0';
	return 1;
}

/* Writes a fix number to the specified buffer.
 *
 * @param[out] buf The buffer to hold the data.
 * @param[in] size Remaining space in bytes within the buffer.
 * @param[in] v The fixed size number to write into the buffer.
 * @param[in] ni Minimum number of integer digits to write number. Maximum will be NMEA_FIX_DECIMAL_DIGITS.
 *    If the integer part uses more digits than stated, more bytes will be written.
 *    More digits than NMEA_FIX_DECIMAL_DIGITS is not possible.
 * @param[in] nd Minimum number of decimal digits to write number. Maximum will be NMEA_FIX_DECIMAL_DIGITS.
 */
/* TODO:TEST */
int write_fix(char * buf, uint32_t size, const struct nmea_fix_t * v, uint32_t ni, uint32_t nd)
{
	char fmt[16];
	uint32_t d = 1;
	uint32_t i;

	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 13) return -1;
	if (nd > NMEA_FIX_DECIMAL_DIGITS) {
		nd = NMEA_FIX_DECIMAL_DIGITS;
	} else if (nd > 0) {
		for (i = nd; i < NMEA_FIX_DECIMAL_DIGITS; ++i) {
			d *= 10;
		}
	}
	snprintf(fmt, sizeof(fmt), "%%%uu.%%0%uu", ni, nd);
	return snprintf(buf, size, fmt, v->i, v->d / d);
}

/* Writers time information in the format HHMMSS to the specified buffer.
 * This format is required by the NMEA data representation.
 * The time is written only if it passes the check_time() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The time information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Time data is wrong.
 */
int write_time(char * buf, uint32_t size, const struct nmea_time_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 6) return -1;
	if (check_time(v)) return -2;
	return snprintf(buf, size, "%02u%02u%02u", v->h, v->m, v->s);
}

/* Writers date information in the format DDMMYY to the specified buffer.
 * This format is required by the NMEA data representation.
 * The date is written only if it passes the check_date() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The date information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Date data is wrong.
 */
int write_date(char * buf, uint32_t size, const struct nmea_date_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 6) return -1;
	if (check_date(v)) return -2;
	return snprintf(buf, size, "%02u%02u%02u", v->d, v->m, v->y % 100);
}

/* Writers latitude information in the format DDMM.SSSS to the specified buffer.
 * This format is required by the NMEA data representation.
 * The latitude is written only if it passes the check_latitude() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The latitude information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Latitude data is wrong.
 */
int write_lat(char * buf, uint32_t size, const struct nmea_angle_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 9) return -1;
	if (check_latitude(v)) return -1;

	/* division by 100 is to achieve 4 decimal digits, maybe it would be better to use write_fix() */
	return snprintf(buf, size, "%02u%02u.%04u", v->d, v->m, (v->s.i * NMEA_FIX_DECIMALS + v->s.d) / 60 / 100);
}

/* Writers longitude information in the format DDDMM.SSSS to the specified buffer.
 * This format is required by the NMEA data representation.
 * The longitude is written only if it passes the check_longitude() criteria.
 *
 * @param[out] buf The buffer to hold the written time.
 * @param[in] size Remaining size in bytes within the buffer.
 * @param[in] v The longitude information.
 * @retval >=0 Number of written bytes to the buffer.
 * @retval -1 Parameter error.
 * @retval -2 Longitude data is wrong.
 */
int write_lon(char * buf, uint32_t size, const struct nmea_angle_t * v)
{
	if (buf == NULL || size == 0 || v == NULL) return -1;
	if (size < 10) return -1;
	if (check_longitude(v)) return -1;

	/* division by 100 is to achieve 4 decimal digits, maybe it would be better to use write_fix() */
	return snprintf(buf, size, "%03u%02u.%04u", v->d, v->m, (v->s.i * NMEA_FIX_DECIMALS + v->s.d) / 60 / 100);
}

/* TODO */
int write_checksum(char * buf, uint32_t size, const char * s, const char * e)
{
	uint8_t sum;
	char str[3];

	if (buf == NULL || size == 0 || s == NULL || e == NULL) return -1;
	sum = checksum(s, e);
	str[0] = i2hex((sum >> 4) & 0x0f);
	str[1] = i2hex((sum >> 0) & 0x0f);
	str[2] = '\0';
	return write_string(buf, size, str);
}

/* Converts a fix point number to float.
 *
 * @param[out] v converted number
 * @param[in] fix the fix point number to convert
 * @retval  0 success
 * @retval -1 failure
 */
int nmea_fix_to_float(float * v, const struct nmea_fix_t * fix)
{
	if (!v || !fix) return -1;
	*v = (float)fix->i + ((float)fix->d / (float)NMEA_FIX_DECIMALS);
	return 0;
}

/* Converts a fix point number to double.
 * @param[out] v converted number
 * @param[in] fix the fix point number to convert
 * @retval  0 success
 * @retval -1 failure
 */
int nmea_fix_to_double(double * v, const struct nmea_fix_t * fix)
{
	if (!v || !fix) return -1;
	*v = (double)fix->i + ((double)fix->d / (double)NMEA_FIX_DECIMALS);
	return 0;
}

/* Returns a pointer to the position of the next comma within the
 * specified string.
 *
 * @param[in] s the string to parse
 * @return the position of the next comma or, if none found, the end of the string
 */
const char * find_token_end(const char * s)
{
	while (s && *s && *s != ',' && *s != '*') ++s;
	return s;
}

/* TODO */
const char * find_sentence_end(const char * s)
{
	while (s && *s && *s != '*') ++s;
	return s;
}

/* TODO
 *
 * @retval 1 token seems to be valid
 * @retval 0 token seems to be invalid
 */
int token_valid(const char * s, const char * p)
{
	return *s && *p && *s != '*' && *p != '*';
}

