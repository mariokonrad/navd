#include <cunit/CUnit.h>
#include <common/endian.h>
#include <seatalk/seatalk.h>
#include <seatalk/seatalk_util.h>

struct test_sentence_t
{
	uint32_t size;
	const char * data;
};

static void test_util_depth_from_meter()
{
	CU_ASSERT_EQUAL(seatalk_depth_from_meter(  100),   32); /*  1 m */
	CU_ASSERT_EQUAL(seatalk_depth_from_meter(  300),   98); /*  3 m */
	CU_ASSERT_EQUAL(seatalk_depth_from_meter(  500),  164); /*  5 m */
	CU_ASSERT_EQUAL(seatalk_depth_from_meter( 1000),  328); /* 10 m */
	CU_ASSERT_EQUAL(seatalk_depth_from_meter( 3000),  984); /* 30 m */
	CU_ASSERT_EQUAL(seatalk_depth_from_meter( 5000), 1640); /* 50 m */
}

static void test_util_depth_to_meter()
{
	CU_ASSERT_EQUAL(seatalk_depth_to_meter(  10),   30); /*   1 ft */
	CU_ASSERT_EQUAL(seatalk_depth_to_meter( 100),  304); /*  10 ft */
	CU_ASSERT_EQUAL(seatalk_depth_to_meter( 300),  914); /*  30 ft */
	CU_ASSERT_EQUAL(seatalk_depth_to_meter( 500), 1524); /*  50 ft */
	CU_ASSERT_EQUAL(seatalk_depth_to_meter(1000), 3048); /* 100 ft */
}

static void test_base_init()
{
	struct seatalk_t data;

	CU_ASSERT_EQUAL(seatalk_init(NULL), -1);

	CU_ASSERT_EQUAL(seatalk_init(&data), 0);
	CU_ASSERT_EQUAL(data.type, 0xff);
}

static void test_sentence_reading_common()
{
	struct seatalk_t info;

	CU_ASSERT_EQUAL(seatalk_read(NULL, NULL, 0), -1);
	CU_ASSERT_EQUAL(seatalk_read(&info, NULL, 0), -1);
	CU_ASSERT_EQUAL(seatalk_read(NULL, "\0\0\0", 0), -1);
	CU_ASSERT_EQUAL(seatalk_read(NULL, NULL, 3), -1);
	CU_ASSERT_EQUAL(seatalk_read(NULL, "\0\0\0", 3), -1);
	CU_ASSERT_EQUAL(seatalk_read(&info, NULL, 3), -1);
}

static void test_sentence_reading_00()
{
	static const struct test_sentence_t SENTENCES[] =
	{
		{ 5, "\x00\x02\x00\x64\x02" },
		{ 5, "\x00\x02\x00\x64\x00" },
		{ 5, "\x00\x02\x00\x00\x02" },
		{ 5, "\x00\x02\x00\x00\x00" },
	};

	static const uint16_t DEPTHS[sizeof(SENTENCES) / sizeof(SENTENCES[0])] =
	{
		612,
		100,
		512,
		0,
	};

	unsigned int i;
	struct seatalk_t info;
	const struct test_sentence_t * s;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		s = &SENTENCES[i];
		CU_ASSERT_EQUAL(seatalk_read(&info, s->data, s->size), 0);
		CU_ASSERT_EQUAL(info.type, SEATALK_DEPTH_BELOW_TRANSDUCER);
		CU_ASSERT_EQUAL(info.sentence.depth_below_transducer.depth, DEPTHS[i]);
	}
}

static void test_sentence_reading_01()
{
	static const struct test_sentence_t SENTENCES[] =
	{
		{ 8, "\x01\x05\x00\x00\x00\x00\x00\x00" },
		{ 8, "\x01\x05\x00\x00\x00\x60\x01\x00" },
		{ 8, "\x01\x05\x04\xba\x20\x28\x01\x00" },
		{ 8, "\x01\x05\x70\x99\x10\x28\x01\x00" },
		{ 8, "\x01\x05\xf3\x18\x00\x26\x0f\x06" },
		{ 8, "\x01\x05\xfa\x03\x00\x30\x07\x03" },
		{ 8, "\x01\x05\xff\xff\xff\xd0\x00\x00" },
	};

	static const struct test_sentence_t IDS[sizeof(SENTENCES) / sizeof(SENTENCES[0])] =
	{
		{ 6, "\x00\x00\x00\x00\x00\x00" }, /* invalid */
		{ 6, "\x00\x00\x00\x60\x01\x00" }, /* Course Computer 400G */
		{ 6, "\x04\xba\x20\x28\x01\x00" }, /* ST60 Tridata */
		{ 6, "\x70\x99\x10\x28\x01\x00" }, /* ST60 Log */
		{ 6, "\xf3\x18\x00\x26\x0f\x06" }, /* ST80 Masterview */
		{ 6, "\xfa\x03\x00\x30\x07\x03" }, /* ST80 Maxi Display */
		{ 6, "\xff\xff\xff\xd0\x00\x00" }, /* Smart Controller Remote Control Handset */
	};

	unsigned int i;
	struct seatalk_t info;
	const struct test_sentence_t * s;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		s = &SENTENCES[i];
		CU_ASSERT_EQUAL(seatalk_read(&info, s->data, s->size), 0);
		CU_ASSERT_EQUAL(info.type, SEATALK_EQUIPMENT_ID);
		CU_ASSERT_EQUAL(memcmp(info.sentence.equipment_id.id, &IDS[i].data, IDS[i].size), 0);
	}
}

static void test_sentence_reading_10()
{
	static const struct test_sentence_t SENTENCES[] =
	{
		{ 4, "\x10\x01\x00\x00" },
		{ 4, "\x10\x01\x10\x00" },
		{ 4, "\x10\x01\x00\x10" },
		{ 4, "\x10\x01\x10\x10" },
	};

	static const uint16_t ANGLES[sizeof(SENTENCES) / sizeof(SENTENCES[0])] =
	{
		0,
		8192,
		32,
		8224,
	};

	unsigned int i;
	struct seatalk_t info;
	const struct test_sentence_t * s;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		s = &SENTENCES[i];
		CU_ASSERT_EQUAL(seatalk_read(&info, s->data, s->size), 0);
		CU_ASSERT_EQUAL(info.type, SEATALK_APPARENT_WIND_ANGLE);
		CU_ASSERT_EQUAL(info.sentence.apparent_wind_angle.angle, ANGLES[i]);
	}
}

static void test_sentence_reading_11()
{
	static const struct test_sentence_t SENTENCES[] =
	{
		{ 4, "\x11\x01\x00\x00" },
		{ 4, "\x11\x01\x01\x00" },
		{ 4, "\x11\x01\x00\x01" },
		{ 4, "\x11\x01\x01\x01" },
		{ 4, "\x11\x01\x00\x00" },
		{ 4, "\x11\x01\x01\x00" },
		{ 4, "\x11\x01\x00\x01" },
		{ 4, "\x11\x01\x01\x01" },
	};

	struct speeds_t
	{
		uint8_t unit;
		uint16_t value;
	};

	static const struct speeds_t SPEEDS[sizeof(SENTENCES) / sizeof(SENTENCES[0])] =
	{
		{ SEATALK_UNIT_KNOT,             0 },
		{ SEATALK_UNIT_KNOT,             0 },
		{ SEATALK_UNIT_KNOT,             0 },
		{ SEATALK_UNIT_KNOT,             0 },
		{ SEATALK_UNIT_METER_PER_SECOND, 0 },
		{ SEATALK_UNIT_METER_PER_SECOND, 0 },
		{ SEATALK_UNIT_METER_PER_SECOND, 0 },
		{ SEATALK_UNIT_METER_PER_SECOND, 0 },
	};

	unsigned int i;
	struct seatalk_t info;
	const struct test_sentence_t * s;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		s = &SENTENCES[i];
		CU_ASSERT_EQUAL(seatalk_read(&info, s->data, s->size), 0);
		CU_ASSERT_EQUAL(info.type, SEATALK_APPARENT_WIND_SPEED);
		CU_ASSERT_EQUAL(info.sentence.apparent_wind_speed.unit, SPEEDS[i].unit);
		CU_ASSERT_EQUAL(info.sentence.apparent_wind_speed.speed, SPEEDS[i].value);
	}
}

static void test_sentence_reading_20()
{
	static const struct test_sentence_t SENTENCES[] =
	{
		{ 4, "\x20\x01\x00\x00" },
		{ 4, "\x20\x01\x10\x00" },
		{ 4, "\x20\x01\x00\x10" },
		{ 4, "\x20\x01\x10\x10" },
	};

	static const uint16_t SPEEDS[sizeof(SENTENCES) / sizeof(SENTENCES[0])] =
	{
		0,
		4096,
		16,
		4112,
	};

	unsigned int i;
	struct seatalk_t info;
	const struct test_sentence_t * s;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		s = &SENTENCES[i];
		CU_ASSERT_EQUAL(seatalk_read(&info, s->data, s->size), 0);
		CU_ASSERT_EQUAL(info.type, SEATALK_SPEED_THROUGH_WATER);
		CU_ASSERT_EQUAL(info.sentence.speed_through_water.speed, SPEEDS[i]);
	}
}

void register_suite_seatalk(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("seatalk", NULL, NULL);
	CU_add_test(suite, "util: depth from meter", test_util_depth_from_meter);
	CU_add_test(suite, "util: depth to meter", test_util_depth_to_meter);
	CU_add_test(suite, "base: init", test_base_init);
	CU_add_test(suite, "sentence reading: common", test_sentence_reading_common);
	CU_add_test(suite, "sentence reading: 00", test_sentence_reading_00);
	CU_add_test(suite, "sentence reading: 01", test_sentence_reading_01);
	CU_add_test(suite, "sentence reading: 10", test_sentence_reading_10);
	CU_add_test(suite, "sentence reading: 11", test_sentence_reading_11);
	CU_add_test(suite, "sentence reading: 20", test_sentence_reading_20);
}

