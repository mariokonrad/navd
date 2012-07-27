#ifndef __FILTER_LIST__H__
#define __FILTER_LIST__H__

#include <navcom/filter.h>

struct filter_desc_list_t {
	size_t num;
	struct filter_desc_t * data;
};

int filterlist_init(struct filter_desc_list_t *);
int filterlist_free(struct filter_desc_list_t *);
int filterlist_append(struct filter_desc_list_t *, const struct filter_desc_t const *);
const struct filter_desc_t const * filterlist_find(const struct filter_desc_list_t const *, const char *);

#endif
