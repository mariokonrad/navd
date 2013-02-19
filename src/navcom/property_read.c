#include <navcom/property_read.h>
#include <stdlib.h>
#include <syslog.h>

int property_read_uint32(const struct property_list_t * properties, const char * key, uint32_t * value)
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

