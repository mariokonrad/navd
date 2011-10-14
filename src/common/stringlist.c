#include <common/stringlist.h>
#include <stdlib.h>

int strlist_init(struct string_list_t * list)
{
	if (list == NULL) return -1;
	list->num = 0;
	list->data = NULL;
	return 0;
}

int strlist_append(struct string_list_t * list, const char * s)
{
	if (list == NULL) return -1;
	if (s == NULL) return -1;
	
	list->num++;
	list->data = realloc(list->data, list->num * sizeof(char *));
	list->data[list->num-1] = strdup(s);
	return 0;
}

int strlist_free(struct string_list_t * list)
{
	size_t i;

	if (list == NULL) return -1;
	if (list->data) {
		for (i = 0; i < list->num; ++i) {
			if (list->data[i]) free(list->data[i]);
		}
		free(list->data);
		list->data = NULL;
	}
	list->num = 0;
	return 0;
}

int strlist_find(const struct string_list_t * list, const char * s)
{
	size_t i;

	if (list == NULL) return -1;
	if (s == NULL) return -1;
	if (list->data == NULL) return 0;
	for (i = 0; i < list->num; ++i) {
		if (strcmp(s, list->data[i]) == 0)
			return 1;
	}
	return 0;
}

