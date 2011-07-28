#ifndef __SERIAL__H__
#define __SERIAL__H__

#include <device/device.h>

struct serial_config_t
{
	char name[128];
};

extern const struct device_operations_t serial_device_operations;

#endif
