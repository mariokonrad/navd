#include <navcom/filter/filter_seatalk_to_nmea.h>
#include <common/macros.h>
#include <string.h>
#include <syslog.h>

static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	UNUSED_ARG(out);
	UNUSED_ARG(ctx);
	UNUSED_ARG(properties);

	if (out == NULL)
		return FILTER_FAILURE;
	if (in == NULL)
		return FILTER_FAILURE;

	if (in->type != MSG_SEATALK) {
		syslog(LOG_WARNING, "no SeaTalk message: %08x", in->type);
		return FILTER_DISCARD;
	}

	/* TODO: implementation */
	return FILTER_DISCARD;
}

static void help(void)
{
	printf("\n");
	printf("filter_seatalk_to_nmea\n");
	printf("\n");
	printf("Converts reveived SeaTalk messages to NMEA messages.\n");
	printf("\n");
	printf("Configuration options:\n");
	printf("\n");
	printf("Example:\n");
	printf("  converter : filter_seatalk_to_nmea {};\n");
	printf("\n");
}

const struct filter_desc_t filter_seatalk_to_nmea = {
	.name = "filter_seatalk_to_nmea",
	.init = NULL,
	.exit = NULL,
	.func = filter,
	.help = help,
};

