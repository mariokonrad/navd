#ifndef __PROPERTY__H__
#define __PROPERTY__H__

struct property_t
{
	char * key;
	char * value;
};

void property_data_free(struct property_t * property);
void property_free(struct property_t * property);

#endif
