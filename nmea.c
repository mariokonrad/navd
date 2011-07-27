#include "nmea.h"
#include <nmea_util.h>
#include <stdio.h>
#include <string.h>
#include <nmea_sentence_gprmb.h>
#include <nmea_sentence_gprmc.h>
#include <nmea_sentence_gpgga.h>
#include <nmea_sentence_gpgsv.h>
#include <nmea_sentence_gpgsa.h>
#include <nmea_sentence_gpgll.h>
#include <nmea_sentence_gpbod.h>
#include <nmea_sentence_gpvtg.h>
#include <nmea_sentence_gprte.h>
#include <nmea_sentence_pgrme.h>
#include <nmea_sentence_pgrmz.h>
#include <nmea_sentence_pgrmm.h>
#include <nmea_sentence_hchdg.h>

static const struct nmea_sentence_t * ALL_SENTENCES[] = {
	&sentence_gprmb,
	&sentence_gprmc,
	&sentence_gpgga,
	&sentence_gpgsv,
	&sentence_gpgsa,
	&sentence_gpgll,
	&sentence_gpbod,
	&sentence_gpvtg,
	&sentence_gprte,
	&sentence_pgrme,
	&sentence_pgrmm,
	&sentence_pgrmz,
	&sentence_hchdg,
};

/* Returns a pointer to the position of the next comma within the
 * specified string.
 *
 * @param[in] s the string to parse
 * @return the position of the next comma or, if none found, the end of the string
 */
static const char * find_token_end(const char * s)
{
	while (s && *s && *s != ',' && *s != '*')  ++s;
	return s;
}

/* TODO
 *
 * @retval 1 token seems to be valid
 * @retval 0 token seems to be invalid
 */
static int token_valid(const char * s, const char * p)
{
	return *s && *p && *s != '*' && *p != '*';
}

/* Parses the sentence using the specified token parser.
 *
 * @param[out] nmea the structure to be filled
 * @param[in] s the sentence to parse
 * @param[in] parser the token parser
 */
static int parse_sentence(struct nmea_t * nmea, const char * s, int (*parse)(int, const char *, const char *, struct nmea_t *))
{
	int state = 0;
	const char * p = s;
	parse(-1, NULL, NULL, nmea);
	for (; token_valid(s, p); s = p+1, ++state) {
		p = find_token_end(s);
		if (parse(state, s, p, nmea)) return -1;
	}
	return 0;
}

/* TODO
 *
 * @param[in]  s read sentence
 * @param[out] nmea data of the parsed structure
 * @param[in] tab table of sentences to parse
 * @retval  0 success
 * @retval  1 unknown sentence
 * @retval -1 parameter error
 * @retval -2 checksum error
 */
int nmea_read_tab(struct nmea_t * nmea, const char * s, const struct nmea_sentence_t ** tab, uint32_t tab_size)
{
	const char * p = s;
	const struct nmea_sentence_t * entry = NULL;
	uint32_t i;

	if (s == NULL || nmea == NULL || tab == NULL || tab_size == 0) return -1;
	if (check_checksum(s, START_TOKEN_NMEA)) return -2;
	if (*s != '$') return -1;
	memset(nmea, 0, sizeof(*nmea));
	++s;
	p = find_token_end(s);
	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->parse && !strncmp(s, entry->tag, p-s)) {
			return parse_sentence(nmea, p+1, entry->parse) ? -1 : 0;
		}
	}
	return 1;
}

/* TODO
 *
 * @param[in]  s read sentence
 * @param[out] nmea data of the parsed structure
 * @retval  0 success
 * @retval  1 unknown sentence
 * @retval -1 parameter error
 * @retval -2 checksum error
 */
int nmea_read(struct nmea_t * nmea, const char * s)
{
	return nmea_read_tab(nmea, s, ALL_SENTENCES, sizeof(ALL_SENTENCES)/sizeof(ALL_SENTENCES[0]));
}

/* Writes the specified NMEA sentence into the buffer. This function
 * handles all NMEA sentences specified in the table.
 * @param[out] buf The buffer to hold the data. This buffer must be large
 *    enough to carry the NMEA sentence.
 * @param[in] size Size of the buffer to hold the data.
 * @param[in] nmea The NMEA data.
 * @param[in] tab Table containing all known or valid NMEA sentences.
 * @retval >= 0 Success, number of bytes written to buffer.
 * @retval -1 Invalid parameters.
 * @retval -2 Unknown NMEA sentence.
 */
int nmea_write_tab(char * buf, uint32_t size, const struct nmea_t * nmea, const struct nmea_sentence_t ** tab, uint32_t tab_size)
{
	const struct nmea_sentence_t * entry;
	uint32_t i;

	if (buf == NULL || size == 0 || nmea == NULL || tab == NULL || tab_size == 0) return -1;
	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->write && entry->type == nmea->type) {
			return entry->write(buf, size, nmea);
		}
	}

	return -2;
}

/* Writes the specified NMEA sentence into the buffer.
 * @param[out] buf The buffer to hold the data. This buffer must be large
 *    enough to carry the NMEA sentence.
 * @param[in] size Size of the buffer to hold the data.
 * @param[in] nmea The NMEA data.
 * @retval >= 0 Success, number of bytes written to buffer.
 * @retval -1 Invalid parameters.
 * @retval -2 Unknown NMEA sentence.
 */
int nmea_write(char * buf, uint32_t size, const struct nmea_t * nmea)
{
	if (buf == NULL || size == 0 || nmea == NULL) return -1;
	return nmea_write_tab(buf, size, nmea, ALL_SENTENCES, sizeof(ALL_SENTENCES)/sizeof(ALL_SENTENCES[0]));
}

