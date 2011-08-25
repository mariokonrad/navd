
D		[0-9]
A		[a-zA-Z_]
H		[0-9a-fA-F]
S		[ ]
WS		[ \t\n]
SPECIAL	[\-.:,;/\+\\\?&@]
SPECIAL1 [\-\+.:/?&@]

%{
#include <config-parser.tab.h>
#include <stdio.h>
#include <string.h>

int line_no = 1;
int char_no = 1;

void log_token(const char * s)
{
	/*printf("<%s>", s);*/
	char_no += strlen(s);
}

int identifier_type(const char * s)
{
	if (strcmp("gps_sim", s) == 0) return SOURCE;
	if (strcmp("gps_serial", s) == 0) return SOURCE;

	if (strcmp("log", s) == 0) return DESTINATION;

	if (strcmp("trn0", s) == 0) return TRANSFORM;

	return IDENTIFIER;
}

%}

%option noyywrap
%option nounput
%option noinput

%%

"->"                          { log_token(yytext); return FORWARD; }
{D}+                          { log_token(yytext); return NUMBER; }
\"({A}|{D}|{S}|{SPECIAL})*\"  { log_token(yytext); return STRING; }
{A}({A}|{D})*                 { log_token(yytext); return identifier_type(yytext); }
"{"                           { log_token(yytext); return '{'; }
"}"                           { log_token(yytext); return '}'; }
"["                           { log_token(yytext); return '['; }
"]"                           { log_token(yytext); return ']'; }
"("                           { log_token(yytext); return '('; }
")"                           { log_token(yytext); return ')'; }
"="                           { log_token(yytext); return '='; }
";"                           { log_token(yytext); return ';'; }
"\n"                          { ++line_no; char_no = 1; }
.                             { ++char_no; }

%%

