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
};

typedef int (*proc_function)(const struct proc_config_t *, const struct property_list_t *);

struct proc_desc_t {
	const char * name;
	proc_function func;
};

volatile int request_terminate;
sigset_t signal_mask;

#endif