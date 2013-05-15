#include <nmea/nmea.h>
#include <stdio.h>

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
#include <nmea/nmea_sentence_iimtw.h>
#include <nmea/nmea_sentence_iimwv.h>
#include <nmea/nmea_sentence_iivwr.h>
#include <nmea/nmea_sentence_iivwt.h>
#include <nmea/nmea_sentence_iidbt.h>
#include <nmea/nmea_sentence_iivlw.h>
#include <nmea/nmea_sentence_iivhw.h>

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
	&sentence_iimtw,
	&sentence_iimwv,
	&sentence_iivwr,
	&sentence_iivwt,
	&sentence_iidbt,
	&sentence_iivlw,
	&sentence_iivhw,
};

/**
 * Reads all known NMEA sentences.
 *
 * @param[in]  s read sentence
 * @param[out] nmea data of the parsed structure
 * @retval  0 success
 * @retval  1 unknown sentence
 * @retval -1 parameter error
 * @retval -2 nmea_checksum error
 * @retval -3 format error
 * @retval -4 unknown sentence
 */
int nmea_read(struct nmea_t * nmea, const char * s)
{
	return nmea_read_tab(nmea, s, SENTENCES, sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *));
}

/**
 * Writes the specified NMEA sentence into the buffer.
 *
 * @param[out] buf The buffer to hold the data. This buffer must be large
 *    enough to carry the NMEA sentence.
 * @param[in] size Size of the buffer to hold the data.
 * @param[in] nmea The NMEA data.
 * @retval >= 0 Success, number of bytes written to buffer.
 * @retval -1 Invalid parameters.
 * @retval -2 Unknown NMEA sentence.
 * @retval -3 Sentence does not support writing.
 */
int nmea_write(char * buf, uint32_t size, const struct nmea_t * nmea)
{
	if (buf == NULL || size == 0 || nmea == NULL) return -1;
	return nmea_write_tab(buf, size, nmea, SENTENCES, sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *));
}

/**
 * Changes the byte order of the NMEA data from host to network byte order.
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

/**
 * Changes the byte order of the NMEA data from network to host byte order.
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

/**
 * Returns the NMEA sentence for the specified sentence type.
 * If an unknown sentence type is defined, NULL will return.
 *
 * @param[in] type The NMEA sentence type.
 * @retval NULL Unknown sentence type.
 * @retval other The NMEA sentence.
 */
const struct nmea_sentence_t * nmea_sentence(uint32_t type)
{
	size_t i;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(struct nmea_sentence_t *); ++i) {
		if (type == SENTENCES[i]->type) {
			return SENTENCES[i];
		}
	}
	return NULL;
}

