#ifndef __NAVCOM__TIMER__H__
#define __NAVCOM__TIMER__H__

#include <navcom/proc.h>
#include <stdint.h>

/**
 * Source specific data.
 */
struct timer_data_t
{
	int initialized;
	uint32_t timer_id;
	struct timespec tm_cfg;
};

extern const struct proc_desc_t timer;

#endif
