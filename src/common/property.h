#ifndef __PROPERTY__H__
#define __PROPERTY__H__

#include <stdio.h>

struct property_t
{
	char * key;
	char * value;
};

struct property_list_t
{
	size_t num;
	struct property_t * data;
};

void property_data_free(struct property_t * property);
void property_free(struct property_t * property);

int proplist_init(struct property_list_t * list);
int proplist_append(struct property_list_t * list, const char * key, const char * value);
int proplist_free(struct property_list_t * list);
int proplist_contains(const struct property_list_t * list, const char * key);
const char * proplist_value(const struct property_list_t * list, const char * key);

#endif
