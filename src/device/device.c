#include <device/device.h>
#include <stdio.h>

/* TODO */
void device_init(struct device_t * device)
{
	if (device == NULL) return;
	device->fd = -1;
	device->data = NULL;
}

