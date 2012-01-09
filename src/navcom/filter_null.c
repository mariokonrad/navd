#include <navcom/filter_null.h>
#include <common/macros.h>
#include <string.h>

static int filter(struct message_t * out, const struct message_t * in, const struct property_list_t * properties)
{
	UNUSED_ARG(properties);

	if (out == NULL) {
		return FILTER_FAILURE;
	}
	if (in == NULL) {
		return FILTER_FAILURE;
	}

	memcpy(out, in, sizeof(struct message_t));
	return FILTER_SUCCESS;
}

const struct filter_desc_t filter_null = {
	"filter_null",
	filter
};

