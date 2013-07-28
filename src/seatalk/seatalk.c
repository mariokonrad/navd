#include <seatalk/seatalk.h>
#include <stdio.h>

#include <seatalk/seatalk_sentence_00.h>
#include <seatalk/seatalk_sentence_01.h>
#include <seatalk/seatalk_sentence_10.h>
#include <seatalk/seatalk_sentence_11.h>
#include <seatalk/seatalk_sentence_20.h>

/**
 * @todo Implementation
 */
static const struct seatalk_sentence_t * SENTENCES[] =
{
	&sentence_00,
	&sentence_01,
	&sentence_10,
	&sentence_11,
	&sentence_20,
};

/**
 * Reads one the known SeaTalk sentences from the specified buffer.
 *
 * @param[out] seatalk The resulting sentence.
 * @param[in] buffer The buffer providing data.
 * @param[in] size Size of the buffer.
 * @retval  0 Success
 * @retval  1 Unknown sentence
 * @retval -1 Parameter error
 *
 * @todo Implementation
 */
int seatalk_read(struct seatalk_t * seatalk, const uint8_t * buffer, uint32_t size)
{
	if (seatalk == NULL)
		return -1;
	if (buffer == NULL)
		return -1;
	if (size == 0)
		return -1;

	return seatalk_read_tab(
		seatalk,
		buffer, size,
		SENTENCES, sizeof(SENTENCES)/sizeof(struct seatalk_sentence_t *));
}

/**
 * @todo Implementation
 * @todo Documentation
 */
int seatalk_write(uint8_t * buffer, uint32_t size, const struct seatalk_t * seatalk)
{
	if (buffer == NULL)
		return -1;
	if (size == 0)
		return -1;
	if (seatalk == NULL)
		return -1;

	return -1;
}

/**
 * @todo Implementation
 * @todo Documentation
 */
int seatalk_hton(struct seatalk_t * seatalk)
{
	if (seatalk == NULL)
		return -1;

	return -1;
}

/**
 * @todo Implementation
 * @todo Documentation
 */
int seatalk_ntoh(struct seatalk_t * seatalk)
{
	if (seatalk == NULL)
		return -1;

	return -1;
}

/**
 * @todo Implementation
 * @todo Documentation
 */
const struct seatalk_sentence_t * seatalk_sentence(uint8_t type)
{
	size_t i;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(struct seatalk_sentence_t *); ++i) {
		if (type == SENTENCES[i]->type) {
			return SENTENCES[i];
		}
	}
	return NULL;
}

