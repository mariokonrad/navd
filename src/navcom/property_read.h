#ifndef __PROPERTY_READ__H__
#define __PROPERTY_READ__H__

#include <stdint.h>
#include <common/property.h>

int property_read_uint32(const struct property_list_t *, const char *, uint32_t *);

#endif
