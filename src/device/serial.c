#include <device/serial.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * Opens a serial device.
 *
 * @param[inout] device The device handling structure.
 * @param[in] cfg The device configuration.
 * @retval  0 Success or file descriptor already valid.
 * @retval -1 Parameter failure.
 */
static int serial_open(
		struct device_t * device,
		const struct device_config_t * cfg)
{
	struct termios old_tio;
	struct termios new_tio;
	const struct serial_config_t * config = NULL;

	if (device == NULL)
		return -1;
	if (cfg == NULL)
		return -1;
	if (device->fd >= 0)
		return 0;

	config = (const struct serial_config_t *)cfg;

	device->fd = open(config->name, O_RDWR | O_NOCTTY);
	if (device->fd < 0)
		return -1;

	tcgetattr(device->fd, &old_tio);
	memset(&new_tio, 0, sizeof(new_tio));
	new_tio.c_cflag = 0
		| B4800
		| CS8
		| CLOCAL
		| CREAD
		;
	new_tio.c_iflag = 0
		| IGNPAR
		;
	new_tio.c_oflag = 0
		;
	new_tio.c_lflag = 0
		;

	new_tio.c_cc[VMIN]  = 1;
	new_tio.c_cc[VTIME] = 0;

	tcflush(device->fd, TCIFLUSH);
	tcsetattr(device->fd, TCSANOW, &new_tio);
	return 0;
}

/**
 * Closes the device, specified by the device handling structure.
 *
 * @param[inout] device The device to be closed.
 * @retval  0 Success.
 * @retval -1 Parameter failure.
 */
static int serial_close(struct device_t * device)
{
	if (device == NULL)
		return -1;
	if (device->fd < 0)
		return 0;
	close(device->fd);
	device->fd = -1;
	return 0;
}

/**
 * Reads the specified amount of bytes from the serial line into the
 * provided buffer.
 *
 * @param[inout] device The device to read from.
 * @param[out] buf The buffer to hold the data.
 * @param[in] size The size of the buffer in bytes.
 * @retval -1 Parameter failure.
 * @return Number of read bytes (might be 0).
 */
static int serial_read(
		struct device_t * device,
		char * buf,
		uint32_t size)
{
	if (device == NULL)
		return -1;
	if (buf == NULL)
		return -1;
	if (device->fd < 0)
		return -1;
	return read(device->fd, buf, size);
}

/**
 * Writes the speicified buffer to the serial line.
 *
 * @param[inout] device The device to write the data to.
 * @param[in] buf The data to write.
 * @param[in] size Number of bytes to write from the buffer to the serial line.
 * @retval -1 Parameter failure.
 * @return Number of written bytes.
 */
static int serial_write(
		struct device_t * device,
		const char * buf,
		uint32_t size)
{
	if (device == NULL)
		return -1;
	if (buf == NULL)
		return -1;
	if (device->fd < 0)
		return -1;
	return write(device->fd, buf, size);
}

/**
 * Exported structure for serial device operations.
 */
const struct device_operations_t serial_device_operations =
{
	.open = serial_open,
	.close = serial_close,
	.read = serial_read,
	.write = serial_write,
};

