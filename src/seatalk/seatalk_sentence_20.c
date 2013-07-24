#include <seatalk/seatalk_sentence_20.h>
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
	struct seatalk_speed_through_water_t * v;

	if (seatalk == NULL)
		return -1;
	if (raw == NULL)
		return -1;

	if (raw->sentence.command != SEATALK_SPEED_THROUGH_WATER)
		return -2;
	if (raw->sentence.attr.length != 1)
		return -2;

	seatalk->type = raw->sentence.command;
	v = &seatalk->sentence.speed_through_water;

	v->speed = 0;
	v->speed += raw->sentence.data[0];
	v->speed <<= 8;
	v->speed += raw->sentence.data[1];

	return 0;
}

const struct seatalk_sentence_t sentence_20 =
{
	.type = SEATALK_SPEED_THROUGH_WATER,
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

