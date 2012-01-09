#include <common/property.h>
#include <stdlib.h>
#include <string.h>

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

int proplist_init(struct property_list_t * list)
{
	if (list == NULL) return -1;
	list->num = 0;
	list->data = NULL;
	return 0;
}

int proplist_append(struct property_list_t * list, const char * key, const char * value)
{
	if (list == NULL) return -1;
	if (key == NULL) return -1;

	list->num++;
	list->data = realloc(list->data, list->num * sizeof(struct property_t));
	list->data[list->num - 1].key = strdup(key);
	if (value) {
		list->data[list->num - 1].value = strdup(value);
	} else {
		list->data[list->num - 1].value = NULL;
	}
	return 0;
}

int proplist_free(struct property_list_t * list)
{
	size_t i;

	if (list == NULL) return -1;
	if (list->data) {
		for (i = 0; i < list->num; ++i) {
			struct property_t * p = &list->data[i];
			if (p->key) free(p->key);
			if (p->value) free(p->value);
		}
		free(list->data);
		list->data = NULL;
	}
	list->num = 0;
	return 0;
}

int proplist_contains(const struct property_list_t * list, const char * key)
{
	size_t i;

	if (list == NULL) return -1;
	if (key == NULL) return -1;
	for (i = 0; i < list->num; ++i) {
		if (strcmp(key, list->data[i].key) == 0)
			return 1;
	}
	return 0;
}

const char * proplist_value(const struct property_list_t * list, const char * key)
{
	size_t i;

	if (list == NULL) return NULL;
	if (key == NULL) return NULL;
	for (i = 0; i < list->num; ++i) {
		if (strcmp(key, list->data[i].key) == 0)
			return list->data[i].value;
	}
	return NULL;
}

