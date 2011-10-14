
%{
#include <config/parser.tab.h>
#include <config/config.h>

int identifier_type(const char * s)
{
	if (config_registered_as_source(s) > 0) return SOURCE_TYPE;
	if (config_registered_as_destination(s) > 0) return DESTINATION_TYPE;
	if (config_registered_as_filter(s) > 0) return FILTER_TYPE;
	return IDENTIFIER;
}

%}

D		[0-9]
A		[a-zA-Z_]
H		[0-9a-fA-F]
S		[ ]
WS		[ \t\n]
SPECIAL	[\-.:,;/\+\\\?&@]
SPECIAL1 [\-\+.:/?&@]

%option yylineno
%option reentrant
%option stack
%option noyywrap
%option nounistd
%option nounput
%option noinput
%option bison-bridge

%x COMMENT

%%

"//"                   { yy_push_state(COMMENT, yyscanner); }
"#"                    { yy_push_state(COMMENT, yyscanner); }

<COMMENT>\n            { yy_pop_state(yyscanner); }
<COMMENT>.             {}

"->"                          { return FORWARD; }
{D}+                          { yylval->str = strdup(yytext); return NUMBER; }
\"({A}|{D}|{S}|{SPECIAL})*\"  { yylval->str = strdup(yytext); return STRING; }
{A}({A}|{D})*                 { yylval->str = strdup(yytext); return identifier_type(yytext); }
"{"                           { return '{'; }
"}"                           { return '}'; }
"["                           { return '['; }
"]"                           { return ']'; }
"("                           { return '('; }
")"                           { return ')'; }
"="                           { return '='; }
";"                           { return ';'; }
":"                           { return ':'; }
"\n"                          { }
[ \t\v\f]                     { }
.                             { }

%%

