#ifndef __NAVCOM__FILTER__H__
#define __NAVCOM__FILTER__H__

#include <navcom/message.h>
#include <common/property.h>

#define FILTER_SUCCESS 0
#define FILTER_FAILURE -1
#define FILTER_DISCARD 1

typedef int (*filter_function)(struct message_t *, const struct message_t *, const struct property_list_t *);

struct filter_desc_t {
	const char * name;
	filter_function func;
};

#endif
