#include <config/config.h>
#include <common/macros.h>
#include <stdlib.h>
#include <string.h>

/* manually copied from lexer.yy.h to prevent compiler warnings */
extern int yylex_init(void * scaner);
extern void yyset_in(FILE * in_str, void * yyscanner);
extern int yylex_destroy(void * yyscanner);
extern int yyparse(void * scanner, struct config_t * config, struct parse_temp_t * tmp);


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
		tmp->dests = realloc(tmp->dests, tmp->num_dests* sizeof(char *));
		tmp->dests[tmp->num_dests-1] = destination;
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
		free(property.key);
		if (property.value != NULL) {
			free(property.value);
		}
	} else {
		tmp->num_props++;
		tmp->props = realloc(tmp->props, tmp->num_props * sizeof(struct property_t));
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

void config_tmp_free(struct parse_temp_t * tmp)
{
	UNUSED_ARG(tmp);

	/* TODO: free memory */
}

void config_free(struct config_t * config)
{
	UNUSED_ARG(config);

	/* TODO: free memory */
}

void config_add_source(struct config_t * config, struct source_t source)
{
	if (config_find_source(config, source.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_sources++;
		config->sources = realloc(config->sources, config->num_sources * sizeof(struct source_t));
		config->sources[config->num_sources-1] = source;
	}
}

void config_add_destination(struct config_t * config, struct destination_t destination)
{
	if (config_find_destination(config, destination.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_destinations++;
		config->destinations = realloc(config->destinations, config->num_destinations * sizeof(struct destination_t));
		config->destinations[config->num_destinations-1] = destination;
	}
}

void config_add_filter(struct config_t * config, struct filter_t filter)
{
	if (config_find_filter(config, filter.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_filters++;
		config->filters = realloc(config->filters, config->num_filters * sizeof(struct filter_t));
		config->filters[config->num_filters-1] = filter;
	}
}

void config_add_route(struct config_t * config, struct route_t route)
{
	/* TODO: prevent duplicates */

	config->num_routes++;
	config->routes = realloc(config->routes, config->num_routes * sizeof(struct route_t));
	config->routes[config->num_routes-1] = route;
}

int config_parse_file(const char * filename, struct config_t * config)
{
	struct parse_temp_t tmp;
	void * scanner;
	FILE * file;
	int rc;

	if (filename == NULL) return EXIT_FAILURE;
	if (config == NULL) return EXIT_FAILURE;

	file = fopen(filename, "r");
	if (file == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	config_init(config);
	config_clear_tmp_property(&tmp);
	config_clear_tmp_dests(&tmp);

	yylex_init(&scanner);
	yyset_in(file, scanner);
	rc = yyparse(scanner, config, &tmp);
	yylex_destroy(scanner);

	fclose(file);
	config_tmp_free(&tmp);

	if (rc < 0) {
		printf("parsing error\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

