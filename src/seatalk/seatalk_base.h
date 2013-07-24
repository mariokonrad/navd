#ifndef __SEATALK_BASE__H__
#define __SEATALK_BASE__H__

#include <stdint.h>
#include <seatalk/seatalk_defs.h>

union seatalk_raw_t
{
	struct data_t
	{
		uint8_t command;
		struct
		{
			uint8_t length : 4;
			uint8_t data   : 4;
		} attr;
		uint8_t data[16];
	} sentence;
	int8_t buffer[sizeof(struct data_t)];
} __attribute((packed));

#define SEATALK_MAX_SENTENCE sizeof(struct seatalk_raw_t)

/**
 * Represents a NMEA message, containing the original raw NMEA sentence
 * and the already (if possible) parsed data.
 */
struct seatalk_t
{
	uint8_t type;
	union seatalk_raw_t raw;
	union
	{
		struct seatalk_depth_below_transducer_t depth_below_transducer;
		struct seatalk_equipment_id_t equipment_id;
		struct seatalk_apparent_wind_angle_t apparent_wind_angle;
		struct seatalk_apparent_wind_speed_t apparent_wind_speed;
		struct seatalk_speed_through_water_t speed_through_water;
		/* TODO: add other SeaTalk sentences */
	} sentence;
} __attribute((packed));

/**
 * Base structure for all implementations of SeaTalk sentences.
 */
struct seatalk_sentence_t
{
	const uint8_t type;
	int (*read)(struct seatalk_t *, const union seatalk_raw_t *);
	int (*write)(union seatalk_raw_t *, const struct seatalk_t *);
	void (*hton)(struct seatalk_t *);
	void (*ntoh)(struct seatalk_t *);
};

int seatalk_init(struct seatalk_t *);

int seatalk_read_tab(
		struct seatalk_t *,
		const char *,
		uint32_t,
		const struct seatalk_sentence_t **,
		uint32_t);

#endif
