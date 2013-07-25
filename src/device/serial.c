#include <device/serial.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static tcflag_t get_baud(const struct serial_config_t * config)
{
	switch (config->baud_rate) {
		case BAUD_300:    return B300;
		case BAUD_600:    return B600;
		case BAUD_1200:   return B1200;
		case BAUD_2400:   return B2400;
		case BAUD_4800:   return B4800;
		case BAUD_9600:   return B9600;
		case BAUD_19200:  return B19200;
		case BAUD_38400:  return B38400;
		case BAUD_57600:  return B57600;
		case BAUD_115200: return B115200;
		case BAUD_230400: return B230400;
		default: break;
	};
	return B0;
}

static tcflag_t get_data_bits(const struct serial_config_t * config)
{
	switch (config->data_bits) {
		case DATA_BIT_5: return CS5;
		case DATA_BIT_6: return CS6;
		case DATA_BIT_7: return CS7;
		case DATA_BIT_8: return CS8;
		default: break;
	}
	return 0;
}

static tcflag_t get_stop_bits(const struct serial_config_t * config)
{
	switch (config->stop_bits) {
		case STOP_BIT_1: return 0;
		case STOP_BIT_2: return CSTOPB;
		default: break;
	}
	return 0;
}

static tcflag_t get_parity_cflag(const struct serial_config_t * config)
{
	switch (config->parity) {
		case PARITY_NONE: return 0;
		case PARITY_EVEN: return PARENB;
		case PARITY_ODD:  return PARENB | PARODD;
		case PARITY_MARK: return 0;
	}
	return 0;
}

static tcflag_t get_parity_iflag(const struct serial_config_t * config)
{
	switch (config->parity) {
		case PARITY_NONE: return IGNPAR;
		case PARITY_EVEN: return INPCK;
		case PARITY_ODD:  return INPCK;
		case PARITY_MARK: return PARMRK;
	}
	return 0;
}

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
		| get_baud(config)
		| get_data_bits(config)
		| get_stop_bits(config)
		| get_parity_cflag(config)
		| CLOCAL /* ignore modem control lines */
		| CREAD /* enable receiver */
		;
	new_tio.c_iflag = 0
		| get_parity_iflag(config)
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

