%{
#include <stdio.h>
#include <stdlib.h>
#include <config/config.h>
#include <common/macros.h>

extern int yyget_lineno(void * yyscanner);

void yyerror(void * scanner, struct config_t * config, struct parse_temp_t * tmp, const char * s)
{
	UNUSED_ARG(config);
	UNUSED_ARG(tmp);

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

