#ifndef __NMEA_SATELITE__H__
#define __NMEA_SATELITE__H__

#include <stdint.h>

/**
 * Information about satelite information when handling NMEA data.
 */
struct nmea_satelite_t {
	uint32_t id;
	uint32_t elevation;
	uint32_t azimuth; /* azimuth against true */
	uint32_t snr;
};

#endif
