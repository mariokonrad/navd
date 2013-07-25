#include <navcom/property_serial.h>
#include <device/serial.h>
#include <common/macros.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int prop_serial_read_device(
		struct serial_config_t * serial,
		const struct property_list_t * properties,
		const char * name
		)
{
	const struct property_t * prop = NULL;

	if (!serial) return EXIT_FAILURE;

	prop = proplist_find(properties, name);
	if (!prop) {
		syslog(LOG_DEBUG, "no device configured, using default '%s'", serial->name);
		return EXIT_SUCCESS;
	}

	strncpy(serial->name, prop->value, sizeof(serial->name));
	syslog(LOG_DEBUG, "device configured, using '%s'", serial->name);
	return EXIT_SUCCESS;
}

int prop_serial_read_baudrate(
		struct serial_config_t * serial,
		const struct property_list_t * properties,
		const char * name
		)
{
	Baud baud;
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	if (!serial) return EXIT_FAILURE;

	prop = proplist_find(properties, name);
	if (!prop) {
		syslog(LOG_DEBUG, "no baud rate configured, using default '%u'", serial->baud_rate);
		return EXIT_SUCCESS;
	}

	baud = (Baud)strtoul(prop->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value for property '%s': '%s'", prop->key, prop->value);
		return EXIT_FAILURE;
	}

	switch (baud) {
		case BAUD_300:
		case BAUD_600:
		case BAUD_1200:
		case BAUD_2400:
		case BAUD_4800:
		case BAUD_9600:
		case BAUD_19200:
		case BAUD_38400:
		case BAUD_57600:
		case BAUD_115200:
		case BAUD_230400:
			serial->baud_rate = baud;
			syslog(LOG_DEBUG, "baud rate configured, using '%u'", serial->baud_rate);
			break;
		default:
			syslog(LOG_ERR, "invalid baud rate configured: %u", baud);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int prop_serial_read_parity(
		struct serial_config_t * serial,
		const struct property_list_t * properties,
		const char * name
		)
{
	const struct property_t * prop = NULL;

	if (!serial) return EXIT_FAILURE;

	prop = proplist_find(properties, name);
	if (!prop) {
		syslog(LOG_DEBUG, "no parity configured, using default '%d'", serial->parity);
		return EXIT_SUCCESS;
	}

	if (strcmp(prop->value, "none") == 0) {
		serial->parity = PARITY_NONE;
	} else if (strcmp(prop->value, "even") == 0) {
		serial->parity = PARITY_EVEN;
	} else if (strcmp(prop->value, "odd") == 0) {
		serial->parity = PARITY_ODD;
	} else if (strcmp(prop->value, "mark") == 0) {
		serial->parity = PARITY_MARK;
	} else {
		syslog(LOG_ERR, "invalid parity configured: %s", prop->value);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int prop_serial_read_databits(
		struct serial_config_t * serial,
		const struct property_list_t * properties,
		const char * name
		)
{
	DataBits bits;
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	if (!serial) return EXIT_FAILURE;

	prop = proplist_find(properties, name);
	if (!prop) {
		syslog(LOG_DEBUG, "no data bits configured, using default '%d'", serial->data_bits);
		return EXIT_SUCCESS;
	}

	bits = (DataBits)strtoul(prop->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value for property '%s': '%s'", prop->key, prop->value);
		return EXIT_FAILURE;
	}

	switch (bits) {
		case DATA_BIT_5:
		case DATA_BIT_6:
		case DATA_BIT_7:
		case DATA_BIT_8:
			serial->data_bits = bits;
			syslog(LOG_DEBUG, "data bits configured, using '%u'", serial->data_bits);
			break;
		default:
			syslog(LOG_ERR, "invalid data bits configured: %u", bits);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int prop_serial_read_stopbits(
		struct serial_config_t * serial,
		const struct property_list_t * properties,
		const char * name
		)
{
	StopBits bits;
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	if (!serial) return EXIT_FAILURE;

	prop = proplist_find(properties, name);
	if (!prop) {
		syslog(LOG_DEBUG, "no stop bits configured, using default '%d'", serial->stop_bits);
		return EXIT_SUCCESS;
	}

	bits = (DataBits)strtoul(prop->value, &endptr, 0);
	if (*endptr != '\0') {
		syslog(LOG_ERR, "invalid value for property '%s': '%s'", prop->key, prop->value);
		return EXIT_FAILURE;
	}

	switch (bits) {
		case STOP_BIT_1:
		case STOP_BIT_2:
			serial->stop_bits = bits;
			syslog(LOG_DEBUG, "stop configured, using '%u'", serial->stop_bits);
			break;
		default:
			syslog(LOG_ERR, "invalid stop bits configured: %u", bits);
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

