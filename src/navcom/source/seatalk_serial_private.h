#ifndef __SEATALK_SERIAL_PRIVATE__H__
#define __SEATALK_SERIAL_PRIVATE__H__

#include <device/serial.h>

/**
 * Source specific data.
 */
struct seatalk_serial_data_t
{
	char type[32];
	union {
		struct serial_config_t serial;
	} config;
};

#endif
