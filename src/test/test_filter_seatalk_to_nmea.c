#include <cunit/CUnit.h>
#include <test_filter_seatalk_to_nmea.h>
#include <navcom/filter/filter_seatalk_to_nmea.h>
#include <common/macros.h>
#include <stdlib.h>

static const struct filter_desc_t * filter = &filter_seatalk_to_nmea;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(filter);
	CU_ASSERT_PTR_NOT_NULL(filter->init);
	CU_ASSERT_PTR_NOT_NULL(filter->exit);
	CU_ASSERT_PTR_NOT_NULL(filter->func);
	CU_ASSERT_PTR_NOT_NULL(filter->help);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(filter->exit(NULL), EXIT_FAILURE);
}

static void test_init_exit(void)
{
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&ctx, 0, sizeof(ctx));

	proplist_init(&properties);

	CU_ASSERT_EQUAL(filter->init(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(filter->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(filter->init(&ctx, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);
	CU_ASSERT_PTR_NOT_NULL(ctx.data);

	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	CU_ASSERT_PTR_NULL(ctx.data);

	proplist_free(&properties);
}

static void test_func_parameter(void)
{
	int rc;
	struct message_t out;
	struct message_t in;
	struct property_list_t properties;

	proplist_init(&properties);

	rc = filter->func(NULL, NULL, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(&out, NULL, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(NULL, &in, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(NULL, NULL, NULL, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	proplist_free(&properties);
}

static void test_func_unsupported(void)
{
	int rc;
	struct message_t out;
	struct message_t in;
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);

	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_SYSTEM;
	rc = filter->func(&out, &in, NULL, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_TIMER;
	rc = filter->func(&out, &in, NULL, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x00, sizeof(in));
	in.type = MSG_NMEA;
	rc = filter->func(&out, &in, NULL, &properties);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_depth_below_transducer(
		uint32_t expected_depth_i,
		uint32_t expected_depth_d,
		uint16_t depth,
		struct filter_context_t * ctx,
		struct property_list_t * properties)
{
	struct message_t out;
	struct message_t in;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));

	/* TODO: calculate meter and fathom */
	uint32_t expected_depth_meter_i = 0;
	uint32_t expected_depth_meter_d = 0;
	uint32_t expected_depth_fathom_i = 0;
	uint32_t expected_depth_fathom_d = 0;

	in.type = MSG_SEATALK;
	in.data.attr.seatalk.type = SEATALK_DEPTH_BELOW_TRANSDUCER;
	in.data.attr.seatalk.sentence.depth_below_transducer.depth = depth;

	CU_ASSERT_EQUAL(filter->func(&out, &in, ctx, properties), FILTER_SUCCESS);

	CU_ASSERT_EQUAL(out.type, MSG_NMEA);
	CU_ASSERT_EQUAL(out.data.attr.nmea.type, NMEA_II_DBT);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_feet.i, expected_depth_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_feet.d, expected_depth_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_unit_feet, NMEA_UNIT_FEET);

	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_meter.i, expected_depth_meter_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_meter.d, expected_depth_meter_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_unit_meter, NMEA_UNIT_METER);

	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_fathom.i, expected_depth_fathom_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_fathom.d, expected_depth_fathom_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_dbt.depth_unit_fathom, NMEA_UNIT_FATHOM);
}

static void test_func_depth_below_transducer_to_dbt(void)
{
	struct message_t out;
	struct message_t in;
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);
	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);

	test_depth_below_transducer(  0,      0,   0, &ctx, &properties);
	test_depth_below_transducer(  0, 500000,   5, &ctx, &properties);
	test_depth_below_transducer(  1,      0,  10, &ctx, &properties);
	test_depth_below_transducer( 10,      0, 100, &ctx, &properties);
	test_depth_below_transducer( 10, 500000, 105, &ctx, &properties);

	CU_ASSERT_EQUAL(filter->func(&out, &in, &ctx, &properties), FILTER_SUCCESS);
	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_apparent_wind_angle(
		uint32_t expected_angle_i,
		uint32_t expected_angle_d,
		uint16_t angle,
		struct filter_context_t * ctx,
		struct property_list_t * properties)
{
	struct message_t out;
	struct message_t in;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));

	in.type = MSG_SEATALK;
	in.data.attr.seatalk.type = SEATALK_APPARENT_WIND_ANGLE;
	in.data.attr.seatalk.sentence.apparent_wind_angle.angle = angle;

	CU_ASSERT_EQUAL(filter->func(&out, &in, ctx, properties), FILTER_SUCCESS);

	CU_ASSERT_EQUAL(out.type, MSG_NMEA);
	CU_ASSERT_EQUAL(out.data.attr.nmea.type, NMEA_II_MWV);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.angle.i, expected_angle_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.angle.d, expected_angle_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.type, NMEA_RELATIVE);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.status, NMEA_STATUS_OK);
}

static void test_func_wind_angle_to_mwv(void)
{
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);
	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);

	test_apparent_wind_angle(  0, 0,   0, &ctx, &properties);
	test_apparent_wind_angle( 90, 0,  90, &ctx, &properties);
	test_apparent_wind_angle(180, 0, 180, &ctx, &properties);
	test_apparent_wind_angle(270, 0, 270, &ctx, &properties);
	test_apparent_wind_angle(359, 0, 359, &ctx, &properties);
	test_apparent_wind_angle(  0, 0, 360, &ctx, &properties);

	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_apparent_wind_speed(
		uint32_t expected_speed_i,
		uint32_t expected_speed_d,
		uint16_t speed,
		struct filter_context_t * ctx,
		struct property_list_t * properties)
{
	struct message_t out;
	struct message_t in;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));

	in.type = MSG_SEATALK;
	in.data.attr.seatalk.type = SEATALK_APPARENT_WIND_SPEED;
	in.data.attr.seatalk.sentence.apparent_wind_speed.unit = SEATALK_UNIT_KNOT;
	in.data.attr.seatalk.sentence.apparent_wind_speed.speed = speed;

	CU_ASSERT_EQUAL(filter->func(&out, &in, ctx, properties), FILTER_SUCCESS);

	CU_ASSERT_EQUAL(out.type, MSG_NMEA);
	CU_ASSERT_EQUAL(out.data.attr.nmea.type, NMEA_II_MWV);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.speed_unit, NMEA_UNIT_KNOT);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.speed.i, expected_speed_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.speed.d, expected_speed_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_mwv.status, NMEA_STATUS_OK);
}

static void test_func_wind_speed_to_mwv(void)
{
	struct message_t out;
	struct message_t in;
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);
	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);

	test_apparent_wind_speed( 0,      0,   0, &ctx, &properties);
	test_apparent_wind_speed( 0, 500000,   5, &ctx, &properties);
	test_apparent_wind_speed( 1,      0,  10, &ctx, &properties);
	test_apparent_wind_speed(10,      0, 100, &ctx, &properties);
	test_apparent_wind_speed(10, 500000, 105, &ctx, &properties);

	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_speed_through_water(
		uint32_t expected_speed_knots_i,
		uint32_t expected_speed_knots_d,
		uint16_t speed_10th_knots,
		struct filter_context_t * ctx,
		struct property_list_t * properties)
{
	struct message_t out;
	struct message_t in;

	uint32_t expected_speed_kmh_i = 0;
	uint32_t expected_speed_kmh_d = 0;
	CU_FAIL(); /* TODO: calculate kmh from knots */

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));

	in.type = MSG_SEATALK;
	in.data.attr.seatalk.type = SEATALK_SPEED_THROUGH_WATER;
	in.data.attr.seatalk.sentence.speed_through_water.speed = speed_10th_knots;

	CU_ASSERT_EQUAL(filter->func(&out, &in, ctx, properties), FILTER_SUCCESS);

	CU_ASSERT_EQUAL(out.type, MSG_NMEA);
	CU_ASSERT_EQUAL(out.data.attr.nmea.type, NMEA_II_VHW);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.degrees_true, NMEA_TRUE);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.speed_knots.i, expected_speed_knots_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.speed_knots.d, expected_speed_knots_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.speed_knots_unit, NMEA_UNIT_KNOT);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.speed_kmh.i, expected_speed_kmh_i);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.speed_kmh.d, expected_speed_kmh_d);
	CU_ASSERT_EQUAL(out.data.attr.nmea.sentence.ii_vhw.speed_kmh_unit, NMEA_UNIT_KMH);
}

static void test_func_speed_through_water_to_vhw(void)
{
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);
	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);

	test_speed_through_water( 0,      0,   0, &ctx, &properties);
	test_speed_through_water( 0, 500000,   5, &ctx, &properties);
	test_speed_through_water( 1,      0,  10, &ctx, &properties);
	test_speed_through_water(10,      0, 100, &ctx, &properties);
	test_speed_through_water(10, 500000, 105, &ctx, &properties);

	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_func_trip_log_to_vlw(void)
{
	struct message_t out;
	struct message_t in;
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);
	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);
	CU_FAIL(); /* TODO: Implement test */
	CU_ASSERT_EQUAL(filter->func(&out, &in, &ctx, &properties), FILTER_SUCCESS);
	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_func_water_temperature_to_mtw(void)
{
	struct message_t out;
	struct message_t in;
	struct filter_context_t ctx;
	struct property_list_t properties;

	memset(&in, 0, sizeof(in));
	memset(&out, 0, sizeof(out));
	memset(&ctx, 0, sizeof(ctx));
	proplist_init(&properties);
	CU_ASSERT_EQUAL(filter->init(&ctx, &properties), EXIT_SUCCESS);
	CU_FAIL(); /* TODO: Implement test */
	CU_ASSERT_EQUAL(filter->func(&out, &in, &ctx, &properties), FILTER_SUCCESS);
	CU_ASSERT_EQUAL(filter->exit(&ctx), EXIT_SUCCESS);
	proplist_free(&properties);
}

void register_suite_filter_seatalk_to_nmea(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, NULL, NULL);
	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "exit", test_exit);
	CU_add_test(suite, "init / exit", test_init_exit);
	CU_add_test(suite, "func: parameter", test_func_parameter);
	CU_add_test(suite, "func: unsupported message types", test_func_unsupported);
	CU_add_test(suite, "func: depth below transducer to DBT", test_func_depth_below_transducer_to_dbt);
	CU_add_test(suite, "func: wind angle to MWV", test_func_wind_angle_to_mwv);
	CU_add_test(suite, "func: wind speed to MWV", test_func_wind_speed_to_mwv);
	CU_add_test(suite, "func: speed through water to VHW", test_func_speed_through_water_to_vhw);
	CU_add_test(suite, "func: trip log to VLW", test_func_trip_log_to_vlw);
	CU_add_test(suite, "func: water temperature to MTW", test_func_water_temperature_to_mtw);
}

