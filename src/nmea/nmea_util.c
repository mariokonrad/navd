#include <nmea/nmea_util.h>
#include <stdio.h>
#include <string.h>

/**
 * Copies the string between [s,e) into v, the user has to make
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

/**
 * Writes a string into the buffer if there is enough room within the buffer.
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

/**
 * Writes a character into the buffer if there is enough room within the buffer.
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

/**
 * Returns a pointer to the position of the next comma within the
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

/**
 * Searches for the end of the NMEA sentence, the character '*'.
 *
 * @param[in] s The string to search for the end.
 * @return Points to the end of the sentence. If the string has ended
 *   before, the result points to the end of the string.
 */
const char * find_sentence_end(const char * s)
{
	while (s && *s && *s != '*') ++s;
	return s;
}

/**
 * Thecks both specified characters to be valid and not '*'.
 *
 * @retval 1 token seems to be valid
 * @retval 0 token seems to be invalid
 */
int token_valid(const char * s, const char * p)
{
	return *s && *p && *s != '*' && *p != '*';
}

