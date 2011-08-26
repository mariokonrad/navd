%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int config_find_tmp_destination(struct parse_temp_t * tmp, char * destination)
{
	size_t i;

	for (i = 0; i < tmp->num_dests; ++i) {
		if (strcmp(destination, tmp->dests[i]) == 0)
			return 1;
	}
	return 0;
}

static int config_find_tmp_propery(struct parse_temp_t * tmp, struct property_t property)
{
	size_t i;

	for (i = 0; i < tmp->num_props; ++i) {
		if (strcmp(property.key, tmp->props[i].key) == 0)
			return 1;
	}
	return 0;
}

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

static void config_clear_tmp_dests(struct parse_temp_t * tmp)
{
	tmp->num_dests = 0;
	tmp->dests = NULL;
}

static void config_add_tmp_destination(struct parse_temp_t * tmp, char * destination)
{
	if (config_find_tmp_destination(tmp, destination)) {
		free(destination);
	} else {
		tmp->num_dests++;
		tmp->dests = realloc(tmp->dests, tmp->num_dests* sizeof(char *));
		tmp->dests[tmp->num_dests-1] = destination;
	}
}

static void config_clear_tmp_property(struct parse_temp_t * tmp)
{
	tmp->num_props = 0;
	tmp->props = NULL;
}

static void config_add_tmp_property(struct parse_temp_t * tmp, struct property_t property)
{
	if (config_find_tmp_propery(tmp, property)) {
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

static void config_tmp_free(struct parse_temp_t * tmp)
{
	/* TODO: free memory */
}

static void config_free(struct config_t * config)
{
	/* TODO: free memory */
}

static void config_add_source(struct config_t * config, struct source_t source)
{
	if (config_find_source(config, source.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_sources++;
		config->sources = realloc(config->sources, config->num_sources * sizeof(struct source_t));
		config->sources[config->num_sources-1] = source;
	}
}

static void config_add_destination(struct config_t * config, struct destination_t destination)
{
	if (config_find_destination(config, destination.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_destinations++;
		config->destinations = realloc(config->destinations, config->num_destinations * sizeof(struct destination_t));
		config->destinations[config->num_destinations-1] = destination;
	}
}

static void config_add_filter(struct config_t * config, struct filter_t filter)
{
	if (config_find_filter(config, filter.name)) {
		/* TODO: prevent duplicates */
	} else {
		config->num_filters++;
		config->filters = realloc(config->filters, config->num_filters * sizeof(struct filter_t));
		config->filters[config->num_filters-1] = filter;
	}
}

static void config_add_route(struct config_t * config, struct route_t route)
{
	/* TODO: prevent duplicates */

	config->num_routes++;
	config->routes = realloc(config->routes, config->num_routes * sizeof(struct route_t));
	config->routes[config->num_routes-1] = route;
}

void yyerror(void * scanner, struct config_t * config, struct parse_temp_t * tmp, const char * s)
{
	printf("\nerror:%d: %s\n", yyget_lineno(scanner), s);
}

%}

%union {
	char * str;
}

%define api.pure
%error-verbose
%lex-param { void * scanner }
%parse-param { void * scanner }
%parse-param { struct config_t * config }
%parse-param { struct parse_temp_t * tmp }

%token <str> IDENTIFIER
%token <str> SOURCE_TYPE
%token <str> DESTINATION_TYPE
%token <str> FILTER_TYPE
%token <str> STRING
%token <str> NUMBER
%token FORWARD

%type <str> value

%start configuration

%%

configuration
	: source configuration
	| destination configuration
	| routing configuration
	| filter configuration
	| source
	| destination
	| routing
	| filter
	;

routing
	: IDENTIFIER FORWARD IDENTIFIER ';'
		{
			struct route_t route;
			route.name_source = $1;
			route.name_filter = NULL;
			route.name_destination = $3;
			config_add_route(config, route);
		}
	| IDENTIFIER FORWARD multiple_destinations ';'
		{
			size_t i;

			for (i = 0; i < tmp->num_dests; ++i) {
				struct route_t route;
				route.name_source = $1;
				route.name_filter = NULL;
				route.name_destination = tmp->dests[i];
				config_add_route(config, route);
			}
			config_clear_tmp_dests(tmp);
		}
	| IDENTIFIER FORWARD '[' IDENTIFIER ']' FORWARD IDENTIFIER ';'
		{
			struct route_t route;
			route.name_source = $1;
			route.name_filter = $4;
			route.name_destination = $7;
			config_add_route(config, route);
		}
	| IDENTIFIER FORWARD '[' IDENTIFIER ']' FORWARD multiple_destinations ';'
		{
			size_t i;

			for (i = 0; i < tmp->num_dests; ++i) {
				struct route_t route;
				route.name_source = $1;
				route.name_filter = $4;
				route.name_destination = tmp->dests[i];
				config_add_route(config, route);
			}
			config_clear_tmp_dests(tmp);
		}
	;

source
	: IDENTIFIER ':' SOURCE_TYPE config ';'
		{
			struct source_t source;
			source.name = $1;
			source.type = $3;
			source.num_properties = tmp->num_props;
			source.properties = tmp->props;
			config_clear_tmp_property(tmp);
			config_add_source(config, source);
		}
	;

destination
	: IDENTIFIER ':' DESTINATION_TYPE config ';'
		{
			struct destination_t destination;
			destination.name = $1;
			destination.type = $3;
			destination.num_properties = tmp->num_props;
			destination.properties = tmp->props;
			config_clear_tmp_property(tmp);
			config_add_destination(config, destination);
		}
	;

filter
	: IDENTIFIER ':' FILTER_TYPE config ';'
		{
			struct filter_t filter;
			filter.name = $1;
			filter.type = $3;
			filter.num_properties = tmp->num_props;
			filter.properties = tmp->props;
			config_clear_tmp_property(tmp);
			config_add_filter(config, filter);
		}
	;

multiple_destinations
	: '(' destination_list ')'
	;

destination_list
	: destination_list IDENTIFIER
		{
			config_add_tmp_destination(tmp, $2);
		}
	| IDENTIFIER
		{
			config_add_tmp_destination(tmp, $1);
		}
	;

config
	: '{' property_list '}'
	| '{' '}'
	;

property_list
	: property_list property
	| property
	;

property
	: IDENTIFIER '=' value
		{
			struct property_t property;
			property.key = $1;
			property.value = $3;
			config_add_tmp_property(tmp, property);
		}
	| IDENTIFIER
		{
			struct property_t property;
			property.key = $1;
			config_add_tmp_property(tmp, property);
		}
	;

value
	: STRING
		{
			$$ = $1;
		}
	| NUMBER
		{
			$$ = $1;
		}
	| IDENTIFIER
		{
			$$ = $1;
		}
	;

%%

/*
int main(int argc, char ** argv)
{
	struct config_t config;
	struct parse_temp_t tmp;
	FILE * file;
	void * scanner;
	size_t i;
	int rc;

	config_init(&config);
	config_clear_tmp_property(&tmp);
	config_clear_tmp_dests(&tmp);

	file = fopen(argv[1], "r");
	if (file == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	yylex_init(&scanner);
	yyset_in(file, scanner);
	rc = yyparse(scanner, &config, &tmp);
	yylex_destroy(scanner);

	fclose(file);

	if (rc < 0) {
		printf("parsing error. exit.\n");
		return EXIT_FAILURE;
	}

	printf("===== SOURCES ========================\n");
	for (i = 0; i < config.num_sources; ++i) {
		struct source_t * p = &config.sources[i];
		printf("  %s : %s\n", p->name, p->type);
	}
	printf("===== DESTINATIONS ===================\n");
	for (i = 0; i < config.num_destinations; ++i) {
		struct destination_t * p = &config.destinations[i];
		printf("  %s : %s\n", p->name, p->type);
	}
	printf("===== FILTERS ========================\n");
	for (i = 0; i < config.num_filters; ++i) {
		struct filter_t * p = &config.filters[i];
		printf("  %s : %s\n", p->name, p->type);
	}
	printf("===== ROUTES =========================\n");
	for (i = 0; i < config.num_routes; ++i) {
		struct route_t * p = &config.routes[i];
		printf("  %s --[%s]--> %s\n", p->name_source, p->name_filter, p->name_destination);
	}

	config_free(&config);
	config_tmp_free(&tmp);
	return EXIT_SUCCESS;
}
*/

