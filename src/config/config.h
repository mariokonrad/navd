#ifndef __CONFIG__H__
#define __CONFIG__H__

#include <stdio.h>

struct property_t
{
	char * key;
	char * value;
};

struct source_t
{
	char * name;
	char * type;
	size_t num_properties;
	struct property_t * properties;
};

struct destination_t
{
	char * name;
	char * type;
	size_t num_properties;
	struct property_t * properties;
};

struct filter_t
{
	char * name;
	char * type;
	size_t num_properties;
	struct property_t * properties;
};

struct route_t
{
	char * name_source;
	char * name_filter;
	char * name_destination;

	struct source_t * source;
	struct filter_t * filter;
	struct destination_t * destination;
};

struct config_t
{
	size_t num_sources;
	struct source_t * sources;
	size_t num_destinations;
	struct destination_t * destinations;
	size_t num_filters;
	struct filter_t * filters;
	size_t num_routes;
	struct route_t * routes;
};

struct parse_temp_t {
	size_t num_props;
	struct property_t * props;
	size_t num_dests;
	char ** dests;
};

int config_find_tmp_destination(struct parse_temp_t * tmp, char * destination);
int config_find_tmp_propery(struct parse_temp_t * tmp, struct property_t property);
int config_find_source(struct config_t * config, const char * source);
int config_find_destination(struct config_t * config, const char * destination);
int config_find_filter(struct config_t * config, const char * filter);
void config_clear_tmp_dests(struct parse_temp_t * tmp);
void config_add_tmp_destination(struct parse_temp_t * tmp, char * destination);
void config_clear_tmp_property(struct parse_temp_t * tmp);
void config_add_tmp_property(struct parse_temp_t * tmp, struct property_t property);
void config_init(struct config_t * config);
void config_tmp_free(struct parse_temp_t * tmp);
void config_free(struct config_t * config);
void config_add_source(struct config_t * config, struct source_t source);
void config_add_destination(struct config_t * config, struct destination_t destination);
void config_add_filter(struct config_t * config, struct filter_t filter);
void config_add_route(struct config_t * config, struct route_t route);

int config_parse_file(const char * filename, struct config_t * config);

#endif
