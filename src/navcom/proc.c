#include <navcom/proc.h>

volatile int request_terminate = 0;
sigset_t signal_mask;

