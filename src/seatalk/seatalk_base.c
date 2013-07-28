#include <seatalk/seatalk_base.h>
#include <common/macros.h>
#include <string.h>

/**
 * Initializes the specified structure.
 *
 * @param[out] seatalk The structgure to initialize.
 * @retval  0 Success
 * @retval -1 Parameter error
 */
int seatalk_init(struct seatalk_t * seatalk)
{
	if (seatalk == NULL)
		return -1;
	memset(seatalk, 0, sizeof(struct seatalk_t));
	seatalk->type = SEATALK_NONE;
	return 0;
}

/**
 * Reads the SeaTalk sentence from the buffer, using the
 * defined sentences in the table.
 *
 * @todo Implementation
 *
 * @param[out] seatalk data of the parsed structure
 * @param[in] buffer Data to read
 * @param[in] size Size of buffer to read.
 * @param[in] tab table of sentences to parse
 * @param[in] tab_size size of the table of sentences to parse
 * @retval  0 success
 * @retval -1 parameter error
 * @retval -4 unknown sentence
 */
int seatalk_read_tab(
		struct seatalk_t * seatalk,
		const uint8_t * buffer,
		uint32_t size,
		const struct seatalk_sentence_t ** tab,
		uint32_t tab_size)
{
	const struct seatalk_sentence_t * entry;
	union seatalk_raw_t raw;
	uint32_t i;
	int rc;

	if (seatalk == NULL)
		return -1;
	if (buffer == NULL)
		return -1;
	if (size == 0)
		return -1;
	if (tab == NULL)
		return -1;
	if (tab_size == 0)
		return -1;

	if (size > sizeof(raw))
		return -1;
	memcpy(raw.buffer, buffer, size);

	for (i = 0; i < tab_size; ++i) {
		entry = tab[i];
		if (entry->read && entry->type == raw.sentence.command) {
			seatalk_init(seatalk);
			rc = entry->read(seatalk, &raw);
			if (rc >= 0)
				memcpy(&seatalk->raw, &raw, sizeof(raw));
			return rc;
		}
	}

	return -4;
}

