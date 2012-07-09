#ifndef __CONFIG__H__
#define __CONFIG__H__

#include <stdio.h>
#include <common/property.h>
#include <common/stringlist.h>

struct proc_t
{
	char * name;
	char * type;
	struct property_list_t properties;
};

struct filter_t
{
	char * name;
	char * type;
	struct property_list_t properties;
};

struct route_t
{
	char * name_source;
	char * name_filter;
	char * name_destination;

	struct proc_t * source;
	struct filter_t * filter;
	struct proc_t * destination;
};

struct config_t
{
	size_t num_sources;
	struct proc_t * sources;
	size_t num_destinations;
	struct proc_t * destinations;
	size_t num_filters;
	struct filter_t * filters;
	size_t num_routes;
	struct route_t * routes;
};

struct parse_temp_t
{
	struct property_list_t properties;
	struct string_list_t destinations;
};

void config_clear_tmp_dests(
		struct parse_temp_t * tmp);

int config_add_tmp_destination(
		struct parse_temp_t * tmp,
		const char * destination);

void config_clear_tmp_property(
		struct parse_temp_t * tmp);

int config_add_tmp_property(
		struct parse_temp_t * tmp,
		const char * key,
		const char * value);

int config_add_source(
		struct config_t * config,
		const char * name,
		const char * type,
		struct property_list_t * properties);

int config_add_destination(
		struct config_t * config,
		const char * name,
		const char * type,
		struct property_list_t * properties);

int config_add_filter(
		struct config_t * config,
		const char * name,
		const char * type,
		struct property_list_t * properties);

void config_add_route(
		struct config_t * config,
		const char * source,
		const char * filter,
		const char * destination);

char * config_strdup(const char *);
char * config_strdup_s(const char *);

void config_free(struct config_t * config);
void config_register_free(void);
int config_register_source(const char * type);
int config_register_destination(const char * type);
int config_register_filter(const char * type);
int config_registered_as_source(const char * s);
int config_registered_as_destination(const char * s);
int config_registered_as_filter(const char * s);
int config_parse_file(const char * filename, struct config_t * config);

#endif
