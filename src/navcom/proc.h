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
typedef void (*help_function)(void);

struct proc_desc_t {
	const char * name;
	prop_function init;
	proc_function exit;
	proc_function func;
	help_function help;
};

void proc_set_request_terminate(int);
int proc_request_terminate(void);

sigset_t * proc_get_signal_mask(void);

#endif
