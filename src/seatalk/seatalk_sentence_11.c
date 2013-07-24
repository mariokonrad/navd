#include <seatalk/seatalk_sentence_11.h>
#include <stdio.h>
#include <string.h>

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
	struct seatalk_apparent_wind_speed_t * v;

	if (seatalk == NULL)
		return -1;
	if (raw == NULL)
		return -1;

	if (raw->sentence.command != SEATALK_APPARENT_WIND_SPEED)
		return -2;
	if (raw->sentence.attr.length != 1)
		return -2;

	seatalk->type = raw->sentence.command;
	v = &seatalk->sentence.apparent_wind_speed;

	v->speed = 0;
	v->speed += raw->sentence.data[0] & 0x7f;
	v->speed += raw->sentence.data[1] & 0x0f;

	v->unit = (raw->sentence.data[0] & 0x80)
		? SEATALK_UNIT_METER_PER_SECOND
		: SEATALK_UNIT_KNOT
		;

	return 0;
}

const struct seatalk_sentence_t sentence_11 =
{
	.type = SEATALK_APPARENT_WIND_SPEED,
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

