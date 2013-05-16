#include <common/stringutil.h>
#include <string.h>
#include <stdlib.h>

/**
 * This function reimplements strdup, because strdup is not C99 standard.
 *
 * @param[in] s String to copy.
 * @return A newly allocated string, a copy of the specified one.
 */
char * stringdup(const char * s)
{
	char * res;

	if (s == NULL) return NULL;
	res = (char *)malloc(strlen(s) + 1);
	if (res) {
		strcpy(res, s);
	}
	return res;
}

