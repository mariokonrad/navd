#ifndef __PROGRAMOPTIONS__H__
#define __PROGRAMOPTIONS__H__

#include <limits.h>

/**
 * This structure contains all possible options the program provides.
 */
struct options_data_t {
	int help;
	char help_specific[128];
	int daemonize;
	int config;
	int dump_config;
	int list;
	int list_compact;
	unsigned int max_msg;
	int log_mask;
	char config_filename[PATH_MAX+1];
};

int parse_options(int, char **, struct options_data_t *);
void print_usage(const char *);

#endif
