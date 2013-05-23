#include <device/simulator.h>
#include <common/macros.h>
#include <stdio.h>
#include <string.h>

/**
 * Structure to hand simulation data.
 */
struct simulator_data_t
{
	const char * s;
	int i;
	int len;
};

/* TODO: Simulator specific data must not be static */
static struct simulator_data_t simulator_data;

/**
 * Opens the simulator device.
 *
 * @param[out] device The device descriptor structure.
 * @param[in] cfg The configuration.
 * @retval -1 Parameter failure.
 * @retval  0 Success.
 */
static int simulator_open(
		struct device_t * device,
		const struct device_config_t * cfg)
{
	UNUSED_ARG(cfg);

	if (device == NULL)
		return -1;
	if (device->fd >= 0)
		return 0;
	device->fd = 0;
	device->data = &simulator_data;
	simulator_data.i = 0;
	simulator_data.s =
		"$GPRMC,202451,A,4702.3966,N,00818.3287,E,0.0,312.3,260711,0.6,E,A*19\r\n"
		"$GPGSV,3,1,12,03,77,155,29,06,65,131,34,07,05,288,00,08,08,314,00*76\r\n"
		"$GPGLL,4702.3966,N,00818.3287,E,202451,A,A*43\r\n"
		"\0"
		;
	simulator_data.len = strlen(simulator_data.s);
	return 0;
}

/**
 * Closes the device.
 *
 * @param[inout] device The device descriptor device.
 * @retval -1 Parameter failure.
 * @retval  0 Success.
 */
static int simulator_close(struct device_t * device)
{
	if (device == NULL)
		return -1;
	if (device->fd < 0)
		return 0;
	device->fd = -1;
	return 0;
}

/**
 * Reads data from the simulator.
 *
 * @param[out] device The device descriptor.
 * @param[out] buf The buffer to contain the read data.
 * @param[in] size Buffer size in bytes.
 * @retval -1 Parameter failure.
 * @return Number of read bytes.
 */
static int simulator_read(
		struct device_t * device,
		char * buf,
		uint32_t size)
{
	struct simulator_data_t * data = NULL;
	uint32_t i;

	if (device == NULL)
		return -1;
	if (buf == NULL)
		return -1;
	if (device->fd < 0)
		return -1;

	data = (struct simulator_data_t *)device->data;
	for (i = 0; i < size; ++i) {
		buf[i] = data->s[data->i];
		data->i = (data->i + 1) % data->len;
	}
	return (int)size;
}

/**
 * Does nothing. Returns always -1.
 */
static int simulator_write(
		struct device_t * device,
		const char * buf,
		uint32_t size)
{
	UNUSED_ARG(device);
	UNUSED_ARG(buf);
	UNUSED_ARG(size);
	return -1;
}

/**
 * Structure to describe the simulator device.
 */
const struct device_operations_t simulator_operations =
{
	.open = simulator_open,
	.close = simulator_close,
	.read = simulator_read,
	.write = simulator_write,
};

