#ifndef __NAVCOM__GPS_SERIAL_PRIVATE__H__
#define __NAVCOM__GPS_SERIAL_PRIVATE__H__

#include <device/serial.h>

/**
 * Source specific data.
 */
struct gps_serial_data_t
{
	struct serial_config_t serial_config;
};

#endif
