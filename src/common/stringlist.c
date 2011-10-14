#include <common/stringlist.h>
#include <stdlib.h>

int strlist_append(struct string_list_t * sl, const char * s)
{
	if (sl == NULL) return -1;
	if (s == NULL) return -1;
	
	sl->num++;
	sl->data = realloc(sl->data, sl->num * sizeof(char *));
	sl->data[sl->num-1] = strdup(s);
	return 0;
}

int strlist_free(struct string_list_t * sl)
{
	size_t i;

	if (sl == NULL) return -1;
	for (i = 0; i < sl->num; ++i) {
		if (sl->data[i]) free(sl->data[i]);
	}
	sl->num = 0;
	return 0;
}

int strlist_find(const struct string_list_t * sl, const char * s)
{
	size_t i;

	for (i = 0; i < sl->num; ++i) {
		if (strcmp(s, sl->data[i]) == 0)
			return 0;
	}
	return -1;
}

