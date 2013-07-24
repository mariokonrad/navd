#ifndef __SEATALK_BASE__H__
#define __SEATALK_BASE__H__

#include <stdint.h>
#include <seatalk/seatalk_defs.h>

struct seatalk_raw_t
{
	uint8_t command;
	struct
	{
		uint8_t data   : 4;
		uint8_t length : 4;
	} attr;
	uint8_t data[16];
} __attribute((packed));

#define SEATALK_MAX_SENTENCE sizeof(struct seatalk_raw_t)

/**
 * Represents a NMEA message, containing the original raw NMEA sentence
 * and the already (if possible) parsed data.
 */
struct seatalk_t
{
	uint8_t type;
	struct seatalk_raw_t raw;
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

#endif
