#include <navcom/filter/filter_nmea.h>
#include <common/macros.h>
#include <string.h>
#include <syslog.h>

static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	const struct nmea_sentence_t * nmea = NULL;

	UNUSED_ARG(ctx);

	if (out == NULL)
		return FILTER_FAILURE;
	if (in == NULL)
		return FILTER_FAILURE;

	if (in->type != MSG_NMEA) {
		syslog(LOG_WARNING, "no NMEA message: %08x", in->type);
		return FILTER_DISCARD;
	}

	if (properties == NULL)
		return FILTER_DISCARD;

	nmea = nmea_sentence(in->data.attr.nmea.type);
	if (nmea == NULL) {
		syslog(LOG_WARNING, "unknown NMEA message type: %08x", in->data.attr.nmea.type);
		return FILTER_DISCARD;
	}

	if (!proplist_contains(properties, nmea->tag))
		return FILTER_DISCARD;

	memcpy(out, in, sizeof(struct message_t));
	return FILTER_SUCCESS;
}

static void help(void)
{
	printf("\n");
	printf("filter_nmea\n");
	printf("\n");
	printf("Forwards all configured NMEA messages, all others are dropped.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("  x : list of tags of NMEA sentences to forward.\n");
	printf("\n");
	printf("Example:\n");
	printf("  rmx_only : filter_nmea { GPRMC GPRMB };\n");
	printf("\n");
}

const struct filter_desc_t filter_nmea = {
	.name = "filter_nmea",
	.init = NULL,
	.exit = NULL,
	.func = filter,
	.help = help,
};

