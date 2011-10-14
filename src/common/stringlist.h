#ifndef __STRINGLIST__H__
#define __STRINGLIST__H__

#include <stdio.h>

struct string_list_t
{
	size_t num;
	char ** data;
};

int strlist_append(struct string_list_t * sl, const char * s);
int strlist_free(struct string_list_t * sl);
int strlist_find(const struct string_list_t * sl, const char * s);

#endif
