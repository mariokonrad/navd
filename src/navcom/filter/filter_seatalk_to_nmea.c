#include <navcom/filter/filter_seatalk_to_nmea.h>
#include <common/macros.h>
#include <string.h>
#include <syslog.h>

/**
 * Filter specific data must contain a copy of following NMEA structures to be filled and copied on demand.
 * This is necessary because most NMEA sentences contain data of more than one SeaTalk message.
 */
struct filter_seatalk_to_nmea_data_t
{
	struct nmea_ii_mwv_t mwv;
	struct nmea_ii_vwr_t vwr;
	struct nmea_ii_vwt_t vwt;
	struct nmea_ii_dbt_t dbt;
	struct nmea_ii_vlw_t vlw;
	struct nmea_ii_vhw_t vhw;
	struct nmea_ii_mtw_t mtw;
};

static int convert_00(
		struct filter_seatalk_to_nmea_data_t * data,
		struct nmea_t * nmea,
		const struct seatalk_t * seatalk)
{
	UNUSED_ARG(nmea);
	UNUSED_ARG(seatalk);

	/* TODO: implementation */

	return FILTER_DISCARD;
}

static int convert_01(
		struct filter_seatalk_to_nmea_data_t * data,
		struct nmea_t * nmea,
		const struct seatalk_t * seatalk)
{
	UNUSED_ARG(nmea);
	UNUSED_ARG(seatalk);

	/* TODO: implementation */

	return FILTER_DISCARD;
}

static int convert_10(
		struct filter_seatalk_to_nmea_data_t * data,
		struct nmea_t * nmea,
		const struct seatalk_t * seatalk)
{
	UNUSED_ARG(nmea);
	UNUSED_ARG(seatalk);

	/* TODO: implementation */

	return FILTER_DISCARD;
}

static int convert_11(
		struct filter_seatalk_to_nmea_data_t * data,
		struct nmea_t * nmea,
		const struct seatalk_t * seatalk)
{
	UNUSED_ARG(nmea);
	UNUSED_ARG(seatalk);

	/* TODO: implementation */

	return FILTER_DISCARD;
}

static int convert_20(
		struct filter_seatalk_to_nmea_data_t * data,
		struct nmea_t * nmea,
		const struct seatalk_t * seatalk)
{
	UNUSED_ARG(nmea);
	UNUSED_ARG(seatalk);

	/* TODO: implementation */

	return FILTER_DISCARD;
}

/**
 * Converts the specified SeaTalk message to the corresponding NMEA.
 *
 * @param[out] nmea The resulting NMEA message.
 * @param[in] seaatalk The source of information.
 * @retval FILTER_SUCCESS Successfully converted the meassage, ready to forward it.
 * @retval FILTER_DISCARD Failure of conversion, discard the message.
 */
static int convert(
		struct filter_seatalk_to_nmea_data_t * data,
		struct nmea_t * nmea,
		const struct seatalk_t * seatalk)
{
	struct conversion_table_entry_t
	{
		uint8_t seatalk_type;
		int (*conversion_func)(
				struct filter_seatalk_to_nmea_data_t * data,
				struct nmea_t *,
				const struct seatalk_t *);
	};

	static const struct conversion_table_entry_t TABLE[] =
	{
		{ SEATALK_DEPTH_BELOW_TRANSDUCER, convert_00 },
		{ SEATALK_EQUIPMENT_ID,           convert_01 },
		{ SEATALK_APPARENT_WIND_ANGLE,    convert_10 },
		{ SEATALK_APPARENT_WIND_SPEED,    convert_11 },
		{ SEATALK_SPEED_THROUGH_WATER,    convert_20 },
	};

	unsigned int i;
	const struct conversion_table_entry_t * entry;

	for (i = 0; i < (sizeof(TABLE) / sizeof(TABLE[0])); ++i) {
		entry = &TABLE[i];
		if ((entry->seatalk_type == seatalk->type) && (entry->conversion_func)) {
			return entry->conversion_func(data, nmea, seatalk);
		}
	}

	return FILTER_DISCARD;
}

static int filter(
		struct message_t * out,
		const struct message_t * in,
		struct filter_context_t * ctx,
		const struct property_list_t * properties)
{
	struct filter_seatalk_to_nmea_data_t * data = NULL;

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

	/* TODO: define 'data' */

	memset(out, 0, sizeof(*out));
	out->type = MSG_NMEA;
	return convert(data, &out->data.attr.nmea, &in->data.attr.seatalk);
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

