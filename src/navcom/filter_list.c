#include <navcom/filter_list.h>
#include <stdlib.h>
#include <string.h>

int filterlist_init(struct filter_desc_list_t * list) /* {{{ */
{
	if (list == NULL) return -1;
	list->num = 0;
	list->data = NULL;
	return 0;
} /* }}} */

int filterlist_free(struct filter_desc_list_t * list) /* {{{ */
{
	if (list == NULL) return -1;
	if (list->data) {
		free(list->data);
		list->data = NULL;
	}
	list->num = 0;
	return 0;
} /* }}} */

int filterlist_append(struct filter_desc_list_t * list, const struct filter_desc_t const * desc) /* {{{ */
{
	if (list == NULL) return -1;
	if (desc == NULL) return -1;

	list->num++;
	list->data = realloc(list->data, list->num * sizeof(struct filter_desc_t));
	list->data[list->num-1] = *desc;
	return 0;
} /* }}} */

const struct filter_desc_t const * filterlist_find(const struct filter_desc_list_t const * list, const char * name) /* {{{ */
{
	size_t i;

	if (list == NULL) return NULL;
	if (name == NULL) return NULL;

	for (i = 0; i < list->num; ++i) {
		if (!strcmp(name, list->data[i].name)) {
			return &list->data[i];
		}
	}
	return NULL;
} /* }}} */

