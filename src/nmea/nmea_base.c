#include <nmea/nmea_base.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_checksum.h>
#include <stdio.h>
#include <string.h>

/**
 * Initializes the specified structure.
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

/**
 * Reads all NMEA senteces defined in the specified table.
 *
 * @param[out] nmea data of the parsed structure
 * @param[in] s read sentence
 * @param[in] tab table of sentences to parse
 * @param[in] tab_size size of the table of sentences to parse
 * @retval  0 success
 * @retval -1 parameter error
 * @retval -2 nmea_checksum error
 * @retval -3 format error
 * @retval -4 unknown sentence
 */
int nmea_read_tab(
		struct nmea_t * nmea,
		const char * s,
		const struct nmea_sentence_t ** tab,
		uint32_t tab_size)
{
	const char * p = s;
	const struct nmea_sentence_t * entry = NULL;
	uint32_t i;
	int rc;

	if (s == NULL || nmea == NULL || tab == NULL || tab_size == 0)
		return -1;
	if (nmea_checksum_check(s, START_TOKEN_NMEA))
		return -2;
	if (*s != START_TOKEN_NMEA)
		return -3;
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
	return -4;
}

/**
 * Writes the specified NMEA sentence into the buffer. This function
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
 * @retval -3 Sentence does not support writing.
 */
int nmea_write_tab(char * buf, uint32_t size, const struct nmea_t * nmea, const struct nmea_sentence_t ** tab, uint32_t tab_size)
{
	const struct nmea_sentence_t * entry;
	uint32_t i;

	if (buf == NULL || size == 0 || nmea == NULL || tab == NULL || tab_size == 0) return -1;
	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->type == nmea->type) {
			if (!entry->write) {
				return -3;
			}
			return entry->write(buf, size, nmea);
		}
	}

	return -2;
}

/**
 * Writes the raw data to the specified buffer (if enough space available).
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

/**
 * Changes the byte order of the NMEA data from host to network byte order.
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

/**
 * Changes the byte order of the NMEA data from network to host byte order.
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

