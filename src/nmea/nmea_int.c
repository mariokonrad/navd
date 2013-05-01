#include <nmea/nmea_int.h>
#include <stdio.h>
#include <ctype.h>

/**
 * Parses an unsigned integer value from a specified string. The string
 * is defined by a start and endpoint (not inclusive).
 * This function reads only unsigned integers in decimal form.
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

