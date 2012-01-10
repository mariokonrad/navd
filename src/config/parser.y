%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
			config_add_route(config, $1, NULL, $3);
		}
	| IDENTIFIER FORWARD multiple_destinations ';'
		{
			size_t i;

			for (i = 0; i < tmp->destinations.num; ++i) {
				config_add_route(config, $1, NULL, tmp->destinations.data[i]);
			}
			config_clear_tmp_dests(tmp);
		}
	| IDENTIFIER FORWARD '[' IDENTIFIER ']' FORWARD IDENTIFIER ';'
		{
			config_add_route(config, $1, $4, $7);
		}
	| IDENTIFIER FORWARD '[' IDENTIFIER ']' FORWARD multiple_destinations ';'
		{
			size_t i;

			for (i = 0; i < tmp->destinations.num; ++i) {
				config_add_route(config, $1, $4, tmp->destinations.data[i]);
			}
			config_clear_tmp_dests(tmp);
		}
	;

source
	: IDENTIFIER ':' SOURCE_TYPE config ';'
		{
			if (config_add_source(config, $1, $3, &tmp->properties) < 0) {
				yyerror(scanner, config, tmp, "source already defined");
				YYABORT;
			}
			config_clear_tmp_property(tmp);
		}
	;

destination
	: IDENTIFIER ':' DESTINATION_TYPE config ';'
		{
			if (config_add_destination(config, $1, $3, &tmp->properties) < 0) {
				yyerror(scanner, config, tmp, "destination already defined");
				YYABORT;
			}
			config_clear_tmp_property(tmp);
		}
	;

filter
	: IDENTIFIER ':' FILTER_TYPE config ';'
		{
			if (config_add_filter(config, $1, $3, &tmp->properties) < 0) {
				yyerror(scanner, config, tmp, "filter is already defined");
				YYABORT;
			} else {
				config_clear_tmp_property(tmp);
			}
		}
	;

multiple_destinations
	: '(' destination_list ')'
	;

destination_list
	: destination_list IDENTIFIER
		{
			if (config_add_tmp_destination(tmp, $2) < 0) {
				yyerror(scanner, config, tmp, "duplicate destination in list");
				YYABORT;
			}
		}
	| IDENTIFIER
		{
			if (config_add_tmp_destination(tmp, $1) < 0) {
				yyerror(scanner, config, tmp, "duplicate destination in list");
				YYABORT;
			}
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
			if (config_add_tmp_property(tmp, $1, $3) < 0) {
				yyerror(scanner, config, tmp, "property already defined");
				YYABORT;
			}
		}
	| IDENTIFIER
		{
			if (config_add_tmp_property(tmp, $1, NULL) < 0) {
				yyerror(scanner, config, tmp, "property already defined");
				YYABORT;
			}
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

