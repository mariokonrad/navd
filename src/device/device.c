#include <device/device.h>
#include <stdio.h>

/**
 * Initializes the specified device structure.
 *
 * @param[out] device Structure to initialize.
 */
void device_init(struct device_t * device)
{
	if (device == NULL) return;
	device->fd = -1;
	device->data = NULL;
}

