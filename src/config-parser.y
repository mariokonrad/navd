%{
#include <stdio.h>

extern int yylex(void);

extern int line_no;
extern int char_no;

int yyerror(char * s)
{
	printf("\nerror:%d:%d: %s\n", line_no, char_no, s);
	return 0;
}

%}

%token IDENTIFIER
%token STRING
%token NUMBER
%token FORWARD
%token SOURCE
%token DESTINATION
%token TRANSFORM

%start configuration

%%

configuration
	: node configuration
	| routing configuration
	| transform configuration
	| node
	| routing
	| transform
	;

routing
	: SOURCE FORWARD DESTINATION ';'
		{
			printf("routing to single destination\n");
		}
	| SOURCE FORWARD multiple_destinations ';'
		{
			printf("routing to multiple destinations\n");
		}
	| SOURCE FORWARD '[' TRANSFORM ']' FORWARD DESTINATION ';'
		{
			printf("routing to single destination with transformation\n");
		}
	| SOURCE FORWARD '[' TRANSFORM ']' FORWARD multiple_destinations ';'
		{
			printf("routing to multiple destinations with transformation\n");
		}
	;

node
	: source
	| destination
	;

source
	: SOURCE config ';'
		{
			printf("source configuration\n");
		}
	;

destination
	: DESTINATION config ';'
		{
			printf("destination configuration\n");
		}
	;

transform
	: TRANSFORM config ';'
		{
			printf("transformation configuration\n");
		}
	;

multiple_destinations
	: '(' destination_list ')'
	;

destination_list
	: destination_list DESTINATION
	| DESTINATION
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
	| IDENTIFIER
	;

value
	: STRING
	| NUMBER
	| IDENTIFIER
	;

%%

