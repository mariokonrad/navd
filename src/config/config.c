#include <config/config.h>
#include <common/macros.h>
#include <common/stringlist.h>
#include <stdlib.h>
#include <string.h>

/* manually copied from lexer.yy.h to prevent compiler warnings */
extern int yylex_init(void * scaner);
extern void yyset_in(FILE * in_str, void * yyscanner);
extern int yylex_destroy(void * yyscanner);
extern int yyparse(void * scanner, struct config_t * config, struct parse_temp_t * tmp);

static struct string_list_t reg_sources = { 0, NULL };
static struct string_list_t reg_destinations = { 0, NULL };
static struct string_list_t reg_filters = { 0, NULL };

int config_find_tmp_destination(struct parse_temp_t * tmp, char * destination)
{
	size_t i;

	for (i = 0; i < tmp->num_dests; ++i) {
		if (strcmp(destination, tmp->dests[i]) == 0)
			return 1;
	}
	return 0;
}

int config_find_tmp_propery(struct parse_temp_t * tmp, struct property_t property)
{
	size_t i;

	for (i = 0; i < tmp->num_props; ++i) {
		if (strcmp(property.key, tmp->props[i].key) == 0)
			return 1;
	}
	return 0;
}

int config_find_source(struct config_t * config, const char * source)
{
	size_t i;

	for (i = 0; i < config->num_sources; ++i) {
		if (strcmp(source, config->sources[i].name) == 0)
			return 1;
	}
	return 0;
}

int config_find_destination(struct config_t * config, const char * destination)
{
	size_t i;

	for (i = 0; i < config->num_destinations; ++i) {
		if (strcmp(destination, config->destinations[i].name) == 0)
			return 1;
	}
	return 0;
}

int config_find_filter(struct config_t * config, const char * filter)
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
	tmp->num_dests = 0;
	tmp->dests = NULL;
}

void config_add_tmp_destination(struct parse_temp_t * tmp, char * destination)
{
	if (config_find_tmp_destination(tmp, destination)) {
		/* TODO: error */
		free(destination);
	} else {
		tmp->num_dests++;
		tmp->dests = realloc(tmp->dests,
			tmp->num_dests * sizeof(char *));
		tmp->dests[tmp->num_dests-1] = strdup(destination);
	}
}

void config_clear_tmp_property(struct parse_temp_t * tmp)
{
	tmp->num_props = 0;
	tmp->props = NULL;
}

void config_add_tmp_property(struct parse_temp_t * tmp, struct property_t property)
{
	if (config_find_tmp_propery(tmp, property)) {
		/* TODO: error */
		property_data_free(&property);
	} else {
		tmp->num_props++;
		tmp->props = realloc(tmp->props,
			tmp->num_props * sizeof(struct property_t));
		tmp->props[tmp->num_props-1] = property;
	}
}

void config_init(struct config_t * config)
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

void config_free_tmp(struct parse_temp_t * tmp)
{
	size_t i;

	if (tmp == NULL) return;

	for (i = 0; i < tmp->num_props; ++i) {
		property_free(&tmp->props[i]);
	}
	for (i = 0; i < tmp->num_dests; ++i) {
		if (tmp->dests[i] == NULL) continue;
		free(tmp->dests[i]);
	}
}

void config_free_source(struct source_t * source)
{
	size_t i;

	if (source == NULL) return;
	if (source->name) free(source->name);
	if (source->type) free(source->type);
	for (i = 0; i < source->num_properties; ++i) {
		property_data_free(&source->properties[i]);
	}
}

void config_free_destination(struct destination_t * destination)
{
	size_t i;

	if (destination == NULL) return;
	if (destination->name) free(destination->name);
	if (destination->type) free(destination->type);
	for (i = 0; i < destination->num_properties; ++i) {
		property_data_free(&destination->properties[i]);
	}
}

void config_free_filter(struct filter_t * filter)
{
	size_t i;

	if (filter == NULL) return;
	if (filter->name) free(filter->name);
	if (filter->type) free(filter->type);
	for (i = 0; i < filter->num_properties; ++i) {
		property_data_free(&filter->properties[i]);
	}
}

void config_free_route(struct route_t * route)
{
	if (route == NULL) return;
	if (route->name_source) free(route->name_source);
	if (route->name_destination) free(route->name_destination);
	if (route->name_filter) free(route->name_filter);
	/* source, filter and destination are not to be deleted */
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

void config_add_source(struct config_t * config, struct source_t source)
{
	if (config_find_source(config, source.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_sources++;
		config->sources = realloc(config->sources,
			config->num_sources * sizeof(struct source_t));
		config->sources[config->num_sources-1] = source;
	}
}

void config_add_destination(struct config_t * config, struct destination_t destination)
{
	if (config_find_destination(config, destination.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_destinations++;
		config->destinations = realloc(config->destinations,
			config->num_destinations * sizeof(struct destination_t));
		config->destinations[config->num_destinations-1] = destination;
	}
}

void config_add_filter(struct config_t * config, struct filter_t filter)
{
	if (config_find_filter(config, filter.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_filters++;
		config->filters = realloc(config->filters,
			config->num_filters * sizeof(struct filter_t));
		config->filters[config->num_filters-1] = filter;
	}
}

void config_add_route(struct config_t * config, struct route_t route)
{
	/* TODO: prevent duplicates */

	config->num_routes++;
	config->routes = realloc(config->routes,
		config->num_routes * sizeof(struct route_t));
	config->routes[config->num_routes-1] = route;
}

int config_register_source(const char * type)
{
	return strlist_append(&reg_sources, type);
}

int config_register_destination(const char * type)
{
	return strlist_append(&reg_destinations, type);
}

int config_register_filter(const char * type)
{
	return strlist_append(&reg_filters, type);
}

int config_registered_as_source(const char * s)
{
	return strlist_find(&reg_sources, s);
}

int config_registered_as_destination(const char * s)
{
	return strlist_find(&reg_destinations, s);
}

int config_registered_as_filter(const char * s)
{
	return strlist_find(&reg_filters, s);
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
	config_clear_tmp_dests(&tmp);

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

	/* TODO: finish links in routes to sources, destinations, filters */

	return 0;
}

