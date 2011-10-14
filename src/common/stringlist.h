#ifndef __STRINGLIST__H__
#define __STRINGLIST__H__

#include <stdio.h>

struct string_list_t
{
	size_t num;
	char ** data;
};

int strlist_init(struct string_list_t * list);
int strlist_append(struct string_list_t * list, const char * s);
int strlist_free(struct string_list_t * list);
int strlist_find(const struct string_list_t * list, const char * s);

#endif
