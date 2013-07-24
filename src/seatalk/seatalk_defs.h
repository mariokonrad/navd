#ifndef __SEATALK_DEFS__H__
#define __SEATALK_DEFS__H__

/* more information at:
 * http://www.thomasknauf.de/seatalk.htm
 */

#define SEATALK_DEPTH_BELOW_TRANSDUCER 0x00
#define SEATALK_EQUIPMENT_ID           0x01
#define SEATALK_APPARENT_WIND_ANGLE    0x10
#define SEATALK_APPARENT_WIND_SPEED    0x11
#define SEATALK_SPEED_THROUGH_WATER    0x20
#define SEATALK_NONE                   0xff

#define SEATALK_UNIT_KNOT             0x01
#define SEATALK_UNIT_METER_PER_SECOND 0x02

/* TODO: define other SeaTalk command types */

/**
 * Depth measurement data
 *
 * (corresponding NMEA sentences: DPT, DBT)
 */
struct seatalk_depth_below_transducer_t
{
	uint8_t anchor_alarm_active;
	uint8_t metric_display_units;
	uint8_t transducer_defective;
	uint8_t depth_alarm_active;
	uint8_t shallow_depth_alarm_active;
	uint16_t depth; /* in 10th of feet, 1 m = 3.2808 feet */
} __attribute__((packed));

/**
 * Equipment ID.
 *
 * 00 00 00 60 01 00  Course Computer 400G
 * 04 BA 20 28 01 00  ST60 Tridata
 * 70 99 10 28 01 00  ST60 Log
 * F3 18 00 26 0F 06  ST80 Masterview
 * FA 03 00 30 07 03  ST80 Maxi Display
 * FF FF FF D0 00 00  Smart Controller Remote Control Handset
 */
struct seatalk_equipment_id_t
{
	uint8_t id[6]; /* TODO: change to uint32_t for easier handling? */
} __attribute__((packed));

/**
 * Wind measurement data: apparent wind angle.
 *
 * (corresponding NMEA sentences: MWV)
 */
struct seatalk_apparent_wind_angle_t
{
	uint16_t angle; /* degrees right of bow */
} __attribute__((packed));

/**
 * Wind measurement data: apparent wind speed.
 *
 * (corresponding NMEA sentences: MWV)
 */
struct seatalk_apparent_wind_speed_t
{
	uint8_t unit;   /* unit of value */
	uint16_t speed; /* wind speed in 10th of unit */
} __attribute__((packed));

/**
 * Speed measurement: speed through water.
 *
 * (corresponding NMEA sentences: VHW)
 */
struct seatalk_speed_through_water_t
{
	uint16_t speed; /* speed in 10th of knots */
} __attribute__((packed));

#endif
