#include <seatalk/seatalk_sentence_01.h>
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
	struct seatalk_equipment_id_t * v;
	unsigned int i;

	if (seatalk == NULL)
		return -1;
	if (raw == NULL)
		return -1;

	if (raw->sentence.command != SEATALK_EQUIPMENT_ID)
		return -2;
	if (raw->sentence.attr.length != 5)
		return -2;

	seatalk->type = raw->sentence.command;
	v = &seatalk->sentence.equipment_id;

	for (i = 0; i < 6; ++i) {
		v->id[i] = raw->sentence.data[i];
	}

	return 0;
}

const struct seatalk_sentence_t sentence_01 =
{
	.type = SEATALK_EQUIPMENT_ID,
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

