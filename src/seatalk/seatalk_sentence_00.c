#include <seatalk/seatalk_sentence_00.h>
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
	union flags_t
	{
		struct
		{
			uint8_t anchor_alarm_active        : 1;
			uint8_t metric_display_units       : 1;
			uint8_t transducer_defective       : 1;
			uint8_t unused                     : 3;
			uint8_t depth_alarm_active         : 1;
			uint8_t shallow_depth_alarm_active : 1;
		} attr;
		uint8_t value;
	} __attribute__((packed));

	union flags_t flags;
	struct seatalk_depth_below_transducer_t * v;

	if (seatalk == NULL)
		return -1;
	if (raw == NULL)
		return -1;

	if (raw->sentence.command != SEATALK_DEPTH_BELOW_TRANSDUCER)
		return -2;
	if (raw->sentence.attr.length != 2)
		return -2;

	seatalk->type = raw->sentence.command;
	v = &seatalk->sentence.depth_below_transducer;

	flags.value = raw->sentence.data[0];
	v->anchor_alarm_active        = flags.attr.anchor_alarm_active;
	v->metric_display_units       = flags.attr.metric_display_units;
	v->transducer_defective       = flags.attr.transducer_defective;
	v->depth_alarm_active         = flags.attr.depth_alarm_active;
	v->shallow_depth_alarm_active = flags.attr.shallow_depth_alarm_active;

	v->depth = 0;
	v->depth += raw->sentence.data[2];
	v->depth <<= 8;
	v->depth += raw->sentence.data[1];

	return 0;
}

const struct seatalk_sentence_t sentence_00 =
{
	.type = SEATALK_DEPTH_BELOW_TRANSDUCER,
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

