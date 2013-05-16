#include <common/stringlist.h>
#include <common/stringutil.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initializes the specified string list.
 *
 * @param[out] list String list to initialize.
 */
int strlist_init(struct string_list_t * list)
{
	if (list == NULL) return -1;
	list->num = 0;
	list->data = NULL;
	return 0;
}

/**
 * Appends the specified string to the list.
 *
 * @param[out] list List to append the string to.
 * @param[in] s String to append.
 * @retval  0 Success.
 * @retval -1 Parameter failure.
 */
int strlist_append(struct string_list_t * list, const char * s)
{
	if (list == NULL) return -1;
	if (s == NULL) return -1;

	list->num++;
	list->data = realloc(list->data, list->num * sizeof(char *));
	list->data[list->num-1] = stringdup(s);
	return 0;
}

/**
 * Frees the string list and all containing string.
 *
 * @param[in] list List to free.
 * @retval  0 Success.
 * @retval -1 Parameter failure.
 */
int strlist_free(struct string_list_t * list)
{
	if (list == NULL) return -1;
	if (list->data) {
		size_t i;
		for (i = 0; i < list->num; ++i) {
			if (list->data[i]) free(list->data[i]);
		}
		free(list->data);
		list->data = NULL;
	}
	list->num = 0;
	return 0;
}

/**
 * Searches for the specified string within the string list.
 *
 * @param[in] list The list to search in.
 * @param[in] s String to find in the list.
 * @retval  0 String not found.
 * @retval  1 String found.
 * @retval -1 Parameter failure.
 */
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

