#ifndef __NAVCOM__PROPERTY_SERIAL__H__
#define __NAVCOM__PROPERTY_SERIAL__H__

#include <device/serial.h>
#include <common/property.h>

int prop_serial_read_device(
		struct serial_config_t *,
		const struct property_list_t *,
		const char *
		);

int prop_serial_read_baudrate(
		struct serial_config_t *,
		const struct property_list_t *,
		const char *
		);

int prop_serial_read_parity(
		struct serial_config_t *,
		const struct property_list_t *,
		const char *
		);

int prop_serial_read_databits(
		struct serial_config_t *,
		const struct property_list_t *,
		const char *
		);

int prop_serial_read_stopbits(
		struct serial_config_t *,
		const struct property_list_t *,
		const char *
		);

#endif
