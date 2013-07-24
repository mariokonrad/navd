#include <seatalk/seatalk.h>
#include <stdio.h>

/**
 * @todo Implementation
 */
static const struct seatalk_sentence_t * SENTENCES[] =
{
};

/**
 * @todo Implementation
 * @todo Documentation
 */
int seatalk_read(struct seatalk_t * seatalk, const char * buffer, uint32_t size)
{
	if (seatalk == NULL)
		return -1;
	if (buffer == NULL)
		return -1;
	if (size == 0)
		return -1;

	return -1;
}

/**
 * @todo Implementation
 * @todo Documentation
 */
int seatalk_write(char * buffer, uint32_t size, const struct seatalk_t * seatalk)
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
	return NULL;
}

