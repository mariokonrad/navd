#include <registry.h>
#include <global_config.h>
#include <navcom/proc_list.h>
#include <navcom/filter_list.h>
#include <config/config.h>
#include <stdio.h>
#include <string.h>

#ifdef ENABLE_FILTER_LUA
	#include <navcom/filter/filter_lua.h>
#endif

#ifdef ENABLE_SOURCE_GPSSERIAL
	#include <navcom/source/gps_serial.h>
#endif

#ifdef ENABLE_SOURCE_GPSSIMULATOR
	#include <navcom/source/gps_simulator.h>
#endif

#ifdef ENABLE_SOURCE_SEATALKSIMULATOR
	#include <navcom/source/seatalk_simulator.h>
#endif

#ifdef ENABLE_SOURCE_SEATALKSERIAL
	#include <navcom/source/seatalk_serial.h>
#endif

#ifdef ENABLE_DESTINATION_LUA
	#include <navcom/destination/dst_lua.h>
#endif

#ifdef ENABLE_SOURCE_LUA
	#include <navcom/source/src_lua.h>
#endif

#include <navcom/filter/filter_null.h>

#ifdef ENABLE_FILTER_NMEA
	#include <navcom/filter/filter_nmea.h>
#endif

#include <navcom/destination/message_log.h>

#ifdef ENABLE_DESTINATION_NMEASERIAL
	#include <navcom/destination/nmea_serial.h>
#endif

#ifdef ENABLE_DESTINATION_LOGBOOK
	#include <navcom/destination/logbook.h>
#endif

#include <navcom/source/timer.h>

/**
 * List of all available sources.
 */
static struct proc_desc_list_t desc_sources;

/**
 * List of all available destinations.
 */
static struct proc_desc_list_t desc_destinations;

/**
 * List of all available filters.
 */
static struct filter_desc_list_t desc_filters;

/**
 * Dumps the registered objects (sources, destinations, filters) to the
 * specified stream.
 *
 * @param[in] compact Switch to turn on compact mode, everything on one line.
 */
void registry_dump(int compact)
{
	size_t i;

	if (!compact)
		printf("Sources:");
	for (i = 0; i < desc_sources.num; ++i)
		printf(" %s", desc_sources.data[i].name);
	if (!compact) {
		printf("\n");
		printf("Destinations:");
	}
	for (i = 0; i < desc_destinations.num; ++i)
		printf(" %s", desc_destinations.data[i].name);
	if (!compact) {
		printf("\n");
		printf("Filters:");
	}
	for (i = 0; i < desc_filters.num; ++i)
		printf(" %s", desc_filters.data[i].name);
	printf("\n");
}

static void print_specific_help(void (*help)(void), const char * name)
{
	if (!help) {
		printf("No help available for '%s'\n", name);
		return;
	}

	help();
}

/**
 * Prints specific help for the specified name which can be a
 * registered source, destination or filter.
 *
 * @param[in] name Name of the registered resource to print specific help for.
 */
void registry_print_help_for(const char * name)
{
	size_t i;

	for (i = 0; i < desc_sources.num; ++i) {
		if (strcmp(name, desc_sources.data[i].name) == 0) {
			print_specific_help(desc_sources.data[i].help, name);
			return;
		}
	}
	for (i = 0; i < desc_destinations.num; ++i) {
		if (strcmp(name, desc_destinations.data[i].name) == 0) {
			print_specific_help(desc_destinations.data[i].help, name);
			return;
		}
	}
	for (i = 0; i < desc_filters.num; ++i) {
		if (strcmp(name, desc_filters.data[i].name) == 0) {
			print_specific_help(desc_filters.data[i].help, name);
			return;
		}
	}
	printf("Nothing registered with name '%s'\n", name);
}

const struct proc_desc_list_t * registry_sources(void)
{
	return &desc_sources;
}

void registry_free(void)
{
	pdlist_free(&desc_sources);
	pdlist_free(&desc_destinations);
	filterlist_free(&desc_filters);
}

void register_sources(void)
{
	size_t i;

	pdlist_init(&desc_sources);

	pdlist_append(&desc_sources, &timer);

#ifdef ENABLE_SOURCE_GPSSERIAL
	pdlist_append(&desc_sources, &gps_serial);
#endif

#ifdef ENABLE_SOURCE_GPSSIMULATOR
	pdlist_append(&desc_sources, &gps_simulator);
#endif

#ifdef ENABLE_SOURCE_SEATALKSIMULATOR
	pdlist_append(&desc_sources, &seatalk_simulator);
#endif

#ifdef ENABLE_SOURCE_SEATALKSERIAL
	pdlist_append(&desc_sources, &seatalk_serial);
#endif

#ifdef ENABLE_SOURCE_LUA
	pdlist_append(&desc_sources, &src_lua);
#endif

	for (i = 0; i < desc_sources.num; ++i) {
		config_register_source(desc_sources.data[i].name);
	}
}

const struct proc_desc_list_t * registry_destinations(void)
{
	return &desc_destinations;
}

void register_destinations(void)
{
	size_t i;

	pdlist_init(&desc_destinations);

	pdlist_append(&desc_destinations, &message_log);

#ifdef ENABLE_DESTINATION_NMEASERIAL
	pdlist_append(&desc_destinations, &nmea_serial);
#endif

#ifdef ENABLE_DESTINATION_LOGBOOK
	pdlist_append(&desc_destinations, &logbook);
#endif

#ifdef ENABLE_DESTINATION_LUA
	pdlist_append(&desc_destinations, &dst_lua);
#endif

	for (i = 0; i < desc_destinations.num; ++i) {
		config_register_destination(desc_destinations.data[i].name);
	}
}

const struct filter_desc_list_t * registry_filters(void)
{
	return &desc_filters;
}

void register_filters(void)
{
	size_t i;

	filterlist_init(&desc_filters);

	filterlist_append(&desc_filters, &filter_null);

#ifdef ENABLE_FILTER_NMEA
	filterlist_append(&desc_filters, &filter_nmea);
#endif

#ifdef ENABLE_FILTER_LUA
	filterlist_append(&desc_filters, &filter_lua);
#endif

	for (i = 0; i < desc_filters.num; ++i) {
		config_register_filter(desc_filters.data[i].name);
	}
}

/**
 * Register known types (sources, destinations, filters)
 */
void registry_register(void)
{
	register_sources();
	register_destinations();
	register_filters();
}

