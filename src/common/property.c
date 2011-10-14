#include <common/property.h>
#include <stdlib.h>

void property_data_free(struct property_t * property)
{
	if (property == NULL) return;
	if (property->key) free(property->key);
	if (property->value) free(property->value);
}

void property_free(struct property_t * property)
{
	if (property == NULL) return;
	property_data_free(property);
	free(property);
}

