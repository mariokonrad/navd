#include <seatalk/seatalk_util.h>
#include <math.h>

/**
 * Converts SeaTalk depth values from 100th of meters.
 *
 * @param[in] m 100th of meters.
 * @return Depth values (tenths of feet)
 */
uint16_t seatalk_depth_from_meter(uint32_t m)
{
	return (uint16_t)floor(((m / 100.0) * 3.2808) * 10.0);
}

/**
 * Converts depths in 100th of meters to SeaTalk depth values.
 *
 * @param[in] f Depth values in 10th of feet.
 * @return Depth in 100th of meters.
 */
uint32_t seatalk_depth_to_meter(uint16_t f)
{
	return (uint32_t)floor(((f / 10.0) / 3.2808) * 100.0);
}

