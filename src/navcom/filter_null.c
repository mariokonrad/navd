#include <navcom/filter_null.h>
#include <common/macros.h>
#include <stdlib.h>

static int filter(struct message_t * out, const struct message_t * in, const struct property_list_t * properties)
{
	UNUSED_ARG(properties);

	if (out == NULL) return EXIT_FAILURE;
	if (in == NULL) return EXIT_FAILURE;

	memcpy(out, in, sizeof(struct message_t));
	return EXIT_SUCCESS;
}

const struct filter_desc_t filter_null = {
	"filter_null",
	filter
};

