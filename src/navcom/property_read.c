#include <navcom/property_read.h>
#include <stdlib.h>
#include <syslog.h>

/**
 * Reads an uin32_t property from the list, given by a key.
 *
 * @param[in] properties The properties to be searched.
 * @param[in] key The key of the property to search.
 * @param[out] value The value read from the property, if found.
 * @retval EXIT_SUCCESS Successful conversion of the found property or
 *   not found and using the default value.
 * @retval EXIT_FAILURE Property found but not able to convert.
 */
int property_read_uint32(
		const struct property_list_t * properties,
		const char * key,
		uint32_t * value)
{
	const struct property_t * prop = NULL;
	char * endptr = NULL;

	prop = proplist_find(properties, key);
	if (prop) {
		*value = strtoul(prop->value, &endptr, 0);
		if (*endptr != '\0') {
			syslog(LOG_ERR, "invalid value in '%s': '%s'", prop->key, prop->value);
			return EXIT_FAILURE;
		}
	} else {
		syslog(LOG_DEBUG, "property '%s' not defined, using default of %u", key, *value);
	}
	return EXIT_SUCCESS;
}

/**
 * Reads a string property from the list, given by a key.
 *
 * @param[in] properties The properties to be searched.
 * @param[in] key The key of the property to search.
 * @param[out] buffer The buffer to contain the string.
 * @param[in] size The size of the result buffer.
 * @retval EXIT_SUCCESS Successful conversion of the found property or
 *   not found and using the default value.
 * @retval EXIT_FAILURE Property found but not able to convert.
 */
int property_read_string(
		const struct property_list_t * properties,
		const char * key,
		char * buffer,
		uint32_t size)
{
	const struct property_t * prop = NULL;

	prop = proplist_find(properties, key);
	if (prop) {
		strncpy(buffer, prop->value, size);
	} else {
		syslog(LOG_DEBUG, "property '%s' not defined, using default of '%s'", key, buffer);
	}
	return EXIT_SUCCESS;
}

