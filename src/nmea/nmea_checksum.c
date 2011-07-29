#include <nmea/nmea_checksum.h>
#include <stdio.h>

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

/* TODO */
int write_checksum(char * buf, uint32_t size, const char * s, const char * e)
{
	uint8_t sum;
	char str[3];

	if (buf == NULL || size == 0 || s == NULL || e == NULL) return -1;
	if (size < 3) return -1;
	sum = checksum(s, e);
	str[0] = i2hex((sum >> 4) & 0x0f);
	str[1] = i2hex((sum >> 0) & 0x0f);
	str[2] = '\0';
	return snprintf(buf, size, "%s", str);
}

