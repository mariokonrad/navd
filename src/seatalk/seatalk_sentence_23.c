#include <seatalk/seatalk_sentence_23.h>
#include <common/macros.h>
#include <stdio.h>

/* TODO: implementation of seatalk sentence */

/**
 * @retval  0 Success
 * @retval -1 Parameter failure
 * @retval -2 Invalid raw data (type, size, etc.)
 */
static int read(
		struct seatalk_t * seatalk,
		const union seatalk_raw_t * raw)
{
	struct seatalk_water_temperature_1_t * v;

	if (seatalk == NULL)
		return -1;
	if (raw == NULL)
		return -1;

	if (raw->sentence.command != SEATALK_WATER_TEMPERATURE_1)
		return -2;
	if (raw->sentence.attr.length != 1)
		return -2;

	seatalk->type = raw->sentence.command;
	v = &seatalk->sentence.water_temperature_1;

	/* TODO: implementation of seatalk sentence */

	return 0;
}

const struct seatalk_sentence_t sentence_23 =
{
	.type = SEATALK_WATER_TEMPERATURE_1,
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

