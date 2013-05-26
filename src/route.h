#ifndef __ROUTE__H__
#define __ROUTE__H__

#include <stddef.h>

struct config_t;
struct proc_config_t;
struct message_t;

void route_init(const struct config_t *);

int route_setup(
		const struct config_t *,
		const struct proc_config_t *,
		size_t,
		size_t);

void route_destroy(const struct config_t *);

int route_msg(
		const struct config_t *,
		const struct proc_config_t *,
		const struct message_t *);

#endif
