#include <nmea/nmea.h>
#include <nmea/nmea_util.h>
#include <stdio.h>
#include <string.h>

#include <nmea/nmea_sentence_gprmb.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <nmea/nmea_sentence_gpgga.h>
#include <nmea/nmea_sentence_gpgsv.h>
#include <nmea/nmea_sentence_gpgsa.h>
#include <nmea/nmea_sentence_gpgll.h>
#include <nmea/nmea_sentence_gpbod.h>
#include <nmea/nmea_sentence_gpvtg.h>
#include <nmea/nmea_sentence_gprte.h>
#include <nmea/nmea_sentence_pgrme.h>
#include <nmea/nmea_sentence_pgrmz.h>
#include <nmea/nmea_sentence_pgrmm.h>
#include <nmea/nmea_sentence_hchdg.h>

static const struct nmea_sentence_t * SENTENCES[] = {
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

/* Initializes the specified structure.
 *
 * @param[out] nmea The structure to initialize
 * @retval 0 Success
 * @retval -1 Parameter error
 */
int nmea_init(struct nmea_t * nmea)
{
	if (nmea == NULL) return -1;
	memset(nmea, 0, sizeof(struct nmea_t));
	nmea->type = NMEA_NONE;
	return 0;
}

/* Reads all NMEA senteces defined defined in the specified table.
 *
 * @param[in] s read sentence
 * @param[out] nmea data of the parsed structure
 * @param[in] tab table of sentences to parse
 * @retval  0 success
 * @retval  1 unknown sentence
 * @retval -1 parameter error
 * @retval -2 checksum error
 * @retval -3 format error
 */
int nmea_read_tab(struct nmea_t * nmea, const char * s, const struct nmea_sentence_t ** tab, uint32_t tab_size)
{
	const char * p = s;
	const struct nmea_sentence_t * entry = NULL;
	uint32_t i;
	int rc;

	if (s == NULL || nmea == NULL || tab == NULL || tab_size == 0) return -1;
	if (check_checksum(s, START_TOKEN_NMEA)) return -2;
	if (*s != START_TOKEN_NMEA) return -3;
	p = find_token_end(s+1);
	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->read && strncmp(s+1, entry->tag, p-s-1) == 0) {
			nmea_init(nmea);
			rc = entry->read(nmea, s+1, find_sentence_end(s+1));
			if (rc >= 0) {
				strncpy(nmea->raw, s, NMEA_MAX_SENTENCE);
			}
			return rc;
		}
	}
	return 1;
}

/* Reads all known NMEA sentences.
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
	return nmea_read_tab(nmea, s, SENTENCES, sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *));
}

/* Writes the specified NMEA sentence into the buffer. This function
 * handles all NMEA sentences specified in the table.
 *
 * @param[out] buf The buffer to hold the data. This buffer must be large
 *    enough to carry the NMEA sentence.
 * @param[in] size Size of the buffer to hold the data.
 * @param[in] nmea The NMEA data.
 * @param[in] tab Table containing all known or valid NMEA sentences.
 * @param[in] tab_size The size of the sentence table.
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
 *
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
	return nmea_write_tab(buf, size, nmea, SENTENCES, sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *));
}

/* Writes the raw data to the specified buffer (if enough space available).
 *
 * @param[out] buf The buffer to hold the data. This buffer must be large
 *    enough to carry the NMEA sentence.
 * @param[in] size Size of the buffer to hold the data.
 * @param[in] nmea The NMEA data.
 * @retval >= 0 Success, number of bytes written to buffer.
 * @retval -1 Invalid parameters.
 */
int nmea_write_raw(char * buf, uint32_t size, const struct nmea_t * nmea)
{
	uint32_t len;

	if (buf == NULL || size == 0 || nmea == NULL) return -1;
	len = strlen(nmea->raw);
	if (len > size) return -1;
	strncpy(buf, nmea->raw, len);
	return (int)len;
}

/* Changes the byte order of the NMEA data from host to network byte order.
 *
 * @param[inout] nmea The data to convert.
 * @param[in] tab Table containing all known or valid NMEA sentences.
 * @param[in] tab_size The size of the sentence table.
 * @retval  0 success
 * @retval -1 parameter failure
 * @retval -2 NMEA sentence not supported
 * @retval -3 conversion not supported
 */
int nmea_hton_tab(struct nmea_t * nmea, const struct nmea_sentence_t ** tab, uint32_t tab_size)
{
	uint32_t i;
	const struct nmea_sentence_t * entry;

	if (nmea == NULL || tab == NULL || tab_size == 0) return -1;
	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->type == nmea->type) {
			if (entry->hton == NULL) return -3;
			entry->hton(nmea);
			return 0;
		}
	}
	return -2;
}

/* Changes the byte order of the NMEA data from network to host byte order.
 *
 * @param[inout] nmea The data to convert.
 * @param[in] tab Table containing all known or valid NMEA sentences.
 * @param[in] tab_size The size of the sentence table.
 * @retval  0 success
 * @retval -1 parameter failure
 * @retval -2 NMEA sentence not supported
 * @retval -3 conversion not supported
 */
int nmea_ntoh_tab(struct nmea_t * nmea, const struct nmea_sentence_t ** tab, uint32_t tab_size)
{
	uint32_t i;
	const struct nmea_sentence_t * entry;

	if (nmea == NULL || tab == NULL || tab_size == 0) return -1;
	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->type == nmea->type) {
			if (entry->ntoh == NULL) return -3;
			entry->ntoh(nmea);
			return 0;
		}
	}
	return -2;
}

/* Changes the byte order of the NMEA data from host to network byte order.
 *
 * @param[inout] nmea The data to convert.
 * @retval  0 success
 * @retval -1 parameter faile
 * @retval -2 NMEA sentence not supported
 * @retval -3 conversion not supported
 */
int nmea_hton(struct nmea_t * nmea)
{
	if (nmea == NULL) return -1;
	return nmea_hton_tab(nmea, SENTENCES, sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *));
}

/* Changes the byte order of the NMEA data from network to host byte order.
 *
 * @param[inout] nmea The data to convert.
 * @retval  0 success
 * @retval -1 parameter faile
 * @retval -2 NMEA sentence not supported
 * @retval -3 conversion not supported
 */
int nmea_ntoh(struct nmea_t * nmea)
{
	if (nmea == NULL) return -1;
	return nmea_ntoh_tab(nmea, SENTENCES, sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *));
}

