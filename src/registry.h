#ifndef __REGISTRY__H__
#define __REGISTRY__H__

struct proc_desc_list_t;
struct filter_desc_list_t;

void registry_dump(int);
void registry_print_help_for(const char *);

const struct proc_desc_list_t * registry_sources(void);
const struct proc_desc_list_t * registry_destinations(void);
const struct filter_desc_list_t * registry_filters(void);

void registry_register(void);
void registry_free(void);

#endif
