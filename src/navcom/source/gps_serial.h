#ifndef __NAVCOM__GPS_SERIAL__H__
#define __NAVCOM__GPS_SERIAL__H__

#include <navcom/proc.h>
#include <device/serial.h>

/**
 * Source specific data.
 */
struct gps_serial_data_t
{
	int initialized;
	struct serial_config_t serial_config;
};

extern const struct proc_desc_t gps_serial;

#endif
