#include <navcom/filter_nmea.h>
#include <common/macros.h>
#include <string.h>
#include <syslog.h>

static int filter(struct message_t * out, const struct message_t * in, const struct property_list_t * properties)
{
	const struct nmea_sentence_t * nmea = NULL;

	if (out == NULL) {
		return FILTER_FAILURE;
	}
	if (in == NULL) {
		return FILTER_FAILURE;
	}

	if (in->type != MSG_NMEA) {
		syslog(LOG_WARNING, "no NMEA message: %08x", in->type);
		return FILTER_DISCARD;
	}

	if (properties == NULL) {
		return FILTER_DISCARD;
	}

	nmea = nmea_sentence(in->data.nmea.type);
	if (nmea == NULL) {
		syslog(LOG_WARNING, "unknown NMEA message type: %08x", in->data.nmea.type);
		return FILTER_DISCARD;
	}

	if (!proplist_contains(properties, nmea->tag)) {
		return FILTER_DISCARD;
	}

	memcpy(out, in, sizeof(struct message_t));
	return FILTER_SUCCESS;
}

const struct filter_desc_t filter_nmea = {
	"filter_nmea",
	filter
};

