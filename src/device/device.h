#ifndef __DEVICE__H__
#define __DEVICE__H__

#include <stdint.h>

/**
 * Structure to handle devices.
 *
 * This structure contains the file descriptor in use and device
 * specific data.
 */
struct device_t
{
	int fd;
	void * data;
};

struct device_config_t;

/**
 * Structure to carry device specific operations.
 */
struct device_operations_t
{
	int (*open)(struct device_t *, const struct device_config_t *);
	int (*close)(struct device_t *);
	int (*read)(struct device_t *, char *, uint32_t);
	int (*write)(struct device_t *, const char *, uint32_t);
};

void device_init(struct device_t * device);

#endif
