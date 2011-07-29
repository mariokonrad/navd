#ifndef __NMEA_INT__H__
#define __NMEA_INT__H__

#include <stdint.h>

const char * parse_int(const char * s, const char * e, uint32_t * v);

#endif
