#include <config/config.h>
#include <common/macros.h>
#include <stdlib.h>
#include <string.h>

/* manually copied from lexer.yy.h to prevent compiler warnings */
extern int yylex_init(void * scaner);
extern void yyset_in(FILE * in_str, void * yyscanner);
extern int yylex_destroy(void * yyscanner);
extern int yyparse(void * scanner, struct config_t * config, struct parse_temp_t * tmp);

static struct {
	struct string_list_t sources;
	struct string_list_t destinations;
	struct string_list_t filters;
} registered = { { 0, NULL }, { 0, NULL }, { 0, NULL } };


static int config_find_source(struct config_t * config, const char * source)
{
	size_t i;

	for (i = 0; i < config->num_sources; ++i) {
		if (strcmp(source, config->sources[i].name) == 0)
			return 1;
	}
	return 0;
}

static int config_find_destination(struct config_t * config, const char * destination)
{
	size_t i;

	for (i = 0; i < config->num_destinations; ++i) {
		if (strcmp(destination, config->destinations[i].name) == 0)
			return 1;
	}
	return 0;
}

static int config_find_filter(struct config_t * config, const char * filter)
{
	size_t i;

	for (i = 0; i < config->num_filters; ++i) {
		if (strcmp(filter, config->filters[i].name) == 0)
			return 1;
	}
	return 0;
}

void config_clear_tmp_dests(struct parse_temp_t * tmp)
{
	strlist_free(&tmp->destinations);
}

void config_add_tmp_destination(struct parse_temp_t * tmp, const char * destination)
{
	int rc;

	rc = strlist_find(&tmp->destinations, destination);
	if (rc < 0) {
		/* TODO: fatal error */
	} else if (rc > 0) {
		/* TODO: error: already existing */
	} else {
		strlist_append(&tmp->destinations, destination);
	}
}

void config_clear_tmp_property(struct parse_temp_t * tmp)
{
	proplist_init(&tmp->properties);
}

void config_add_tmp_property(struct parse_temp_t * tmp, const char * key, const char * value)
{
	int rc;

	rc = proplist_contains(&tmp->properties, key);
	if (rc < 0) {
		/* TODO: fatal error */
	} else if (rc > 0) {
		/* TODO: error: already existing key */
	} else {
		proplist_append(&tmp->properties, key, value);
	}
}

static void config_init(struct config_t * config)
{
	config->num_sources = 0;
	config->num_destinations = 0;
	config->num_filters = 0;
	config->num_routes = 0;
	config->sources = NULL;
	config->destinations = NULL;
	config->filters = NULL;
	config->routes = NULL;
}

static void config_free_tmp(struct parse_temp_t * tmp)
{
	if (tmp == NULL) return;
	proplist_free(&tmp->properties);
	strlist_free(&tmp->destinations);
}

static void config_free_source(struct source_t * source)
{
	if (source == NULL) return;
	if (source->name) free(source->name);
	if (source->type) free(source->type);
	proplist_free(&source->properties);
}

static void config_free_destination(struct destination_t * destination)
{
	if (destination == NULL) return;
	if (destination->name) free(destination->name);
	if (destination->type) free(destination->type);
	proplist_free(&destination->properties);
}

static void config_free_filter(struct filter_t * filter)
{
	if (filter == NULL) return;
	if (filter->name) free(filter->name);
	if (filter->type) free(filter->type);
	proplist_free(&filter->properties);
}

static void config_free_route(struct route_t * route)
{
	if (route == NULL) return;
	if (route->name_source) free(route->name_source);
	if (route->name_destination) free(route->name_destination);
	if (route->name_filter) free(route->name_filter);
	/* source, filter and destination are not to be deleted, they are just links */
}

void config_free(struct config_t * config)
{
	size_t i;

	if (config == NULL) return;

	for (i = 0; i < config->num_sources; ++i) {
		config_free_source(&config->sources[i]);
	}
	for (i = 0; i < config->num_destinations; ++i) {
		config_free_destination(&config->destinations[i]);
	}
	for (i = 0; i < config->num_filters; ++i) {
		config_free_filter(&config->filters[i]);
	}
	for (i = 0; i < config->num_routes; ++i) {
		config_free_route(&config->routes[i]);
	}
}

void config_add_source(struct config_t * config, const char * name, const char * type, struct property_list_t * properties)
{
	struct source_t * source;

	if (name == NULL || type == NULL || properties == NULL) {
		/* TODO: fatal error */
	} else if (config_find_source(config, name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_sources++;
		config->sources = realloc(config->sources,
			config->num_sources * sizeof(struct source_t));
		source = &config->sources[config->num_sources-1];
		source->name = strdup(name);
		source->type = strdup(type);
		source->properties = *properties;
	}
}

void config_add_destination(struct config_t * config, const char * name, const char * type, struct property_list_t * properties)
{
	struct destination_t * destination;

	if (name == NULL || type == NULL || properties == NULL) {
		/* TODO: fatal error */
	} else if (config_find_destination(config, name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_destinations++;
		config->destinations = realloc(config->destinations,
			config->num_destinations * sizeof(struct destination_t));
		destination = &config->destinations[config->num_destinations-1];
		destination->name = strdup(name);
		destination->type = strdup(type);
		destination->properties = *properties;
	}
}

void config_add_filter(struct config_t * config, const char * name, const char * type, struct property_list_t * properties)
{
	struct filter_t * filter;

	if (name == NULL || type == NULL || properties == NULL) {
		/* TODO: fatal error */
	} else if (config_find_filter(config, name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_filters++;
		config->filters = realloc(config->filters,
			config->num_filters * sizeof(struct filter_t));
		filter = &config->filters[config->num_filters-1];
		filter->name = strdup(name);
		filter->type = strdup(type);
		filter->properties = *properties;
	}
}

void config_add_route(struct config_t * config, const char * source, const char * filter, const char * destination)
{
	struct route_t * route;

	if (source == NULL || destination == NULL) {
		/* TODO: fatal error */
	} else {
		/* TODO: prevent duplicates */

		config->num_routes++;
		config->routes = realloc(config->routes,
			config->num_routes * sizeof(struct route_t));
		route = &config->routes[config->num_routes-1];
		route->name_source = strdup(source);
		route->name_destination = strdup(destination);
		if (filter) {
			route->name_filter = strdup(filter);
		} else {
			route->name_filter = NULL;
		}
	}
}

int config_register_source(const char * type)
{
	return strlist_append(&registered.sources, type);
}

int config_register_destination(const char * type)
{
	return strlist_append(&registered.destinations, type);
}

int config_register_filter(const char * type)
{
	return strlist_append(&registered.filters, type);
}

int config_registered_as_source(const char * s)
{
	return strlist_find(&registered.sources, s);
}

int config_registered_as_destination(const char * s)
{
	return strlist_find(&registered.destinations, s);
}

int config_registered_as_filter(const char * s)
{
	return strlist_find(&registered.filters, s);
}

static void finish_routes_linking(struct config_t * config)
{
	size_t i;
	size_t j;
	struct route_t * route;

	for (i = 0; i < config->num_routes; ++i) {
		route = &config->routes[i];
		for (j = 0; j < config->num_sources; ++j) {
			if (strcmp(route->name_source, config->sources[j].name) == 0) {
				route->source = &config->sources[j];
				break;
			}
		}
		for (j = 0; j < config->num_destinations; ++j) {
			if (strcmp(route->name_destination, config->destinations[j].name) == 0) {
				route->destination = &config->destinations[j];
				break;
			}
		}
		if (route->name_filter) {
			for (j = 0; j < config->num_filters; ++j) {
				if (strcmp(route->name_filter, config->filters[j].name) == 0) {
					route->filter = &config->filters[j];
					break;
				}
			}
		}
	}
}

/**
 * Parses the configuration file.
 *
 * \param[in] filename The name of the configuration file. This must be the
 *   entire path, absoute or relative.
 * \param[out] config The data structure to be filled with all the parsed
 *   information.
 * \retval  0 Parsing of the configuration file was successful.
 * \retval -1 Error, invalid parameters.
 * \retval -2 Error, not possible to open the file.
 * \retval -3 Error, syntax error.
 */
int config_parse_file(const char * filename, struct config_t * config)
{
	struct parse_temp_t tmp;
	void * scanner;
	FILE * file;
	int rc;

	if (filename == NULL) return -1;
	if (config == NULL) return -1;

	file = fopen(filename, "r");
	if (file == NULL) {
		perror("fopen");
		return -2;
	}

	config_init(config);
	config_clear_tmp_property(&tmp);
	strlist_init(&tmp.destinations);

	yylex_init(&scanner);
	yyset_in(file, scanner);
	rc = yyparse(scanner, config, &tmp);
	yylex_destroy(scanner);

	fclose(file);
	config_free_tmp(&tmp);

	if (rc < 0) {
		printf("parsing error\n");
		return -3;
	}

	finish_routes_linking(config);
	return 0;
}

