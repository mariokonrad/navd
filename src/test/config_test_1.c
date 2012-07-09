#include <stdio.h>
#include <stdlib.h>
#include <config/config.h>
#include <common/macros.h>

int main(int argc, char ** argv)
{
	UNUSED_ARG(argc);
	UNUSED_ARG(argv);

	config_register_source("gps_sim");

	config_register_destination("message_log");

	config_register_filter("filter_null");
	config_register_filter("filter_nmea");

	config_register_free();
	return EXIT_SUCCESS;
}

