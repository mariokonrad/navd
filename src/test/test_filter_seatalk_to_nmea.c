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

static void test_func_wind_angle_to_mwv(void)
{
	/* TODO: Implement test */
	CU_FAIL();
}

static void test_func_wind_speed_to_mwv(void)
{
	/* TODO: Implement test */
	CU_FAIL();
}

static void test_func_depth_below_transducer_to_dbt(void)
{
	/* TODO: Implement test */
	CU_FAIL();
}

static void test_func_speed_through_water_to_vhw(void)
{
	/* TODO: Implement test */
	CU_FAIL();
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
	CU_add_test(suite, "func: wind angle to MWV", test_func_wind_angle_to_mwv);
	CU_add_test(suite, "func: wind speed to MWV", test_func_wind_speed_to_mwv);
	CU_add_test(suite, "func: depth below transducer to DBT", test_func_depth_below_transducer_to_dbt);
	CU_add_test(suite, "func: speed through water to VHW", test_func_speed_through_water_to_vhw);
}

