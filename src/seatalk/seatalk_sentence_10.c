#include <seatalk/seatalk_sentence_10.h>
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
	struct seatalk_apparent_wind_angle_t * v;

	if (seatalk == NULL)
		return -1;
	if (raw == NULL)
		return -1;

	if (raw->sentence.command != SEATALK_APPARENT_WIND_ANGLE)
		return -2;
	if (raw->sentence.attr.length != 1)
		return -2;

	seatalk->type = raw->sentence.command;
	v = &seatalk->sentence.apparent_wind_angle;

	v->angle = 0;
	v->angle += raw->sentence.data[0];
	v->angle <<= 8;
	v->angle += raw->sentence.data[1];

	v->angle *= 2;

	return 0;
}

const struct seatalk_sentence_t sentence_10 =
{
	.type = SEATALK_APPARENT_WIND_ANGLE,
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

