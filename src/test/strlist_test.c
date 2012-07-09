#include <stdlib.h>
#include <common/stringlist.h>
#include <common/macros.h>

int main(int argc, char ** argv)
{
	struct string_list_t s;

	UNUSED_ARG(argc);
	UNUSED_ARG(argv);

	strlist_init(&s);

	strlist_append(&s, "Babs");
	strlist_append(&s, "Mario");
	strlist_append(&s, "Gecko");

	strlist_free(&s);

	return EXIT_SUCCESS;
}

