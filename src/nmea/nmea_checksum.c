#include <nmea/nmea_checksum.h>
#include <stdio.h>

/**
 * Converts the specified hexadecimal character to its numerical representation.
 * If the character does not match a hexadecimal digit, 0xff will return.
 */
static uint8_t hex2i(char c)
{
	if ((c >= '0') && (c <= '9')) return c - '0';
	if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
	return 0xff;
}

/**
 * Returns the hexadecimal digit for the specified value.
 */
static char i2hex(int i)
{
	static const char * TAB = "0123456789ABCDEF";
	return (i >= 0 && i <= 15) ? TAB[i] : '\0';
}

/**
 * Calculates and returns the nmea_checksum between the two positions of a string.
 * The nmea_checksum is calculated inclusive start token and exclusive the end
 * token. If either one is wrong 0x00 will return.
 */
uint8_t nmea_checksum(const char * s, const char * e)
{
	uint8_t chk = 0x00;
	for (; s && e && *s && (s < e); ++s) {
		chk ^= *s;
	}
	return chk;
}

/**
 * Checks the nmea_checksum of the sentence.
 *
 * @param[in] s sentence to check
 * @param[in] start_token The start token to check.
 * @retval  0 success
 * @retval -1 failure
 */
int nmea_checksum_check(const char * s, char start_token)
{
	uint8_t chk = 0;
	if (!s || !(*s) || *s != start_token) return -1;
	++s; /* skip start token */
	for (; *s && *s != '*'; ++s) {
		chk ^= *s;
	}
	++s; /* skip '*' */
	return chk == hex2i(s[0]) * 16 + hex2i(s[1]) ? 0 : -1;
}

/**
 * Writes the checksum, computed from the specified string, into the
 * buffer.
 *
 * @param[out] buf The buffer to contain the checksum in ASCII form.
 * @param[in] size Size of the buffer. Must be at least 3 characters,
 *     terminating zero will be added.
 * @param[in] s Start of the string to compute the checksum for.
 * @param[in] e End of the string.
 * @retval -1 Parameter failure.
 * @return Number of characters written into the specified buffer.
 */
int nmea_checksum_write(char * buf, uint32_t size, const char * s, const char * e)
{
	uint8_t sum;
	char str[3];

	if (buf == NULL || size == 0 || s == NULL || e == NULL) return -1;
	if (size < 3) return -1;
	sum = nmea_checksum(s, e);
	str[0] = i2hex((sum >> 4) & 0x0f);
	str[1] = i2hex((sum >> 0) & 0x0f);
	str[2] = '\0';
	return snprintf(buf, size, "%s", str);
}

