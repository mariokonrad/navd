#include <stdio.h>
#include <stdlib.h>

extern int yyparse();

int main(int argc, char ** argv)
{
	printf("\n");
	yyparse();
	printf("\n");
	return EXIT_SUCCESS;
}

