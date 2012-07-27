#ifndef __PROC_LIST__H__
#define __PROC_LIST__H__

#include <navcom/proc.h>

struct proc_desc_list_t {
	size_t num;
	struct proc_desc_t * data;
};

int pdlist_init(struct proc_desc_list_t *);
int pdlist_free(struct proc_desc_list_t *);
int pdlist_append(struct proc_desc_list_t *, const struct proc_desc_t const *);
const struct proc_desc_t const * pdlist_find(const struct proc_desc_list_t const *, const char *);

#endif
