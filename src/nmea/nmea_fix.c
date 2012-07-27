#include <nmea/nmea_fix.h>
#include <common/endian.h>
#include <stdio.h>
#include <ctype.h>

/**
 * Checks whether all members of the fix number are zero.
 *
 * @retval 0 All members are zero.
 * @retval -1 Parameter error
 * @retval -2 Not all members are zero
 */
int nmea_fix_check_zero(const struct nmea_fix_t * v)
{
	if (v == NULL) return -1;
	return (1
		&& (v->i == 0)
		&& (v->d == 0)
		) ? 0 : -2;
}

/**
 * Parses a fix point number between start and end of the string into the
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
const char * nmea_fix_parse(const char * s, const char * e, struct nmea_fix_t * v)
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

/**
 * Writes a fix number to the specified buffer.
 *
 * @param[out] buf The buffer to hold the data.
 * @param[in] size Remaining space in bytes within the buffer.
 * @param[in] v The fixed size number to write into the buffer.
 * @param[in] ni Minimum number of integer digits to write number. Maximum will be NMEA_FIX_DECIMAL_DIGITS.
 *    If the integer part uses more digits than stated, more bytes will be written.
 *    More digits than NMEA_FIX_DECIMAL_DIGITS is not possible.
 * @param[in] nd Minimum number of decimal digits to write number. Maximum will be NMEA_FIX_DECIMAL_DIGITS.
 * @return The number of characters written into the buffer.
 */
int nmea_fix_write(char * buf, uint32_t size, const struct nmea_fix_t * v, uint32_t ni, uint32_t nd)
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

/**
 * Converts a fix point number to float.
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

/**
 * Converts a fix point number to double.
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

/**
 * @todo Documenation
 */
void nmea_fix_hton(struct nmea_fix_t * v)
{
	if (v == NULL) return;
	v->i = endian_hton_32(v->i);
	v->d = endian_hton_32(v->d);
}

/**
 * @todo Documenation
 */
void nmea_fix_ntoh(struct nmea_fix_t * v)
{
	if (v == NULL) return;
	v->i = endian_ntoh_32(v->i);
	v->d = endian_ntoh_32(v->d);
}

