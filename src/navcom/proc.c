#include <navcom/proc.h>

static volatile int request_terminate = 0;

sigset_t signal_mask;

void proc_set_request_terminate(int value)
{
	request_terminate = value;
}

int proc_request_terminate(void)
{
	return request_terminate;
}

void proc_config_init(struct proc_config_t * ptr)
{
	ptr->pid = -1;
	ptr->rfd = -1;
	ptr->wfd = -1;
	ptr->cfg = NULL;
	ptr->data = NULL;
}

