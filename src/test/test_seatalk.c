#include <cunit/CUnit.h>
#include <common/endian.h>
#include <seatalk/seatalk.h>
#include <seatalk/seatalk_util.h>

struct test_sentence_t
{
	uint32_t size;
	const char * data;
};

static const struct test_sentence_t SENTENCES[] =
{
	/* depth below transducer */
	{ 5, "\x00\x02\x00\x64\x02" },
	{ 5, "\x00\x02\x00\x64\x00" },
	{ 5, "\x00\x02\x00\x00\x02" },
	{ 5, "\x00\x02\x00\x00\x00" },
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

static void test_sentence_reading()
{
	struct seatalk_t info;
	unsigned int i;
	int rc;

	CU_ASSERT_EQUAL(seatalk_read(NULL, NULL, 0), -1);
	CU_ASSERT_EQUAL(seatalk_read(&info, NULL, 0), -1);
	CU_ASSERT_EQUAL(seatalk_read(NULL, "\0\0\0", 0), -1);
	CU_ASSERT_EQUAL(seatalk_read(NULL, NULL, 3), -1);
	CU_ASSERT_EQUAL(seatalk_read(NULL, "\0\0\0", 3), -1);
	CU_ASSERT_EQUAL(seatalk_read(&info, NULL, 3), -1);

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		memset(&info, 0, sizeof(info));
		rc = seatalk_read(&info, SENTENCES[i].data, SENTENCES[i].size);
		CU_ASSERT_EQUAL(rc, 0);
	}
}

void register_suite_seatalk(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("seatalk", NULL, NULL);
	CU_add_test(suite, "util: depth from meter", test_util_depth_from_meter);
	CU_add_test(suite, "util: depth to meter", test_util_depth_to_meter);
	CU_add_test(suite, "base: init", test_base_init);
	CU_add_test(suite, "sentence: reading", test_sentence_reading);
}

