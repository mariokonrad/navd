#ifndef __NAVCOM__PROC__H__
#define __NAVCOM__PROC__H__

#include <config/config.h>
#include <common/property.h>
#include <signal.h>

struct proc_config_t {
	int pid; /* process id */
	int rfd; /* pipe file descriptor to read */
	int wfd; /* pipe file descriptor to write */

	const struct proc_t const * cfg; /* configuration */
	void * data; /* proc specific data */
};

void proc_config_init(struct proc_config_t *);

typedef int (*prop_function)(struct proc_config_t *, const struct property_list_t *);
typedef int (*proc_function)(struct proc_config_t *);

struct proc_desc_t {
	const char * name;
	prop_function configure;
	proc_function func;
	proc_function clean;
};

void proc_set_request_terminate(int);
int proc_request_terminate(void);

extern sigset_t signal_mask;

#endif
