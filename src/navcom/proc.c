#include <navcom/proc.h>

volatile int request_terminate = 0;
sigset_t signal_mask;

void proc_config_init(struct proc_config_t * ptr)
{
	ptr->pid = -1;
	ptr->rfd = -1;
	ptr->wfd = -1;
	ptr->cfg = NULL;
	ptr->data = NULL;
}

