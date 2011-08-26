
%{
#include <config/parser.tab.h>

int identifier_type(const char * s)
{
	if (strcmp("gps_sim", s) == 0) return SOURCE_TYPE;
	if (strcmp("gps_serial", s) == 0) return SOURCE_TYPE;

	if (strcmp("message_log", s) == 0) return DESTINATION_TYPE;

	if (strcmp("message_filter", s) == 0) return FILTER_TYPE;

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

