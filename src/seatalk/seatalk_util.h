#ifndef __SEATALK_UTIL__H__
#define __SEATALK_UTIL__H__

#include <stdint.h>

uint16_t seatalk_depth_from_meter(uint32_t);
uint32_t seatalk_depth_to_meter(uint16_t);

#endif
