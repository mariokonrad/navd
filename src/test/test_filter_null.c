#include <cunit/CUnit.h>
#include <test_filter_null.h>
#include <navcom/filter/filter_null.h>
#include <common/macros.h>
#include <string.h>

static const struct filter_desc_t * filter = &filter_null;

static void test_configure(void)
{
	CU_ASSERT_EQUAL(filter->configure, NULL);
}

static void test_free_ctx(void)
{
	CU_ASSERT_EQUAL(filter->free_ctx, NULL);
}

static void test_func(void)
{
	int rc;
	struct message_t out;
	struct message_t in;

	memset(&out, 0x00, sizeof(out));
	memset(&in, 0x55, sizeof(in));

	rc = filter->func(NULL, NULL, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(&out, NULL, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(NULL, &in, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_FAILURE);

	rc = filter->func(&out, &in, NULL, NULL);
	CU_ASSERT_EQUAL(rc, FILTER_SUCCESS);

	CU_ASSERT_EQUAL(memcmp(&out, &in, sizeof(out)), 0);
}

void register_suite_filter_null(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, NULL, NULL);
	CU_add_test(suite, "configure", test_configure);
	CU_add_test(suite, "free_ctx", test_free_ctx);
	CU_add_test(suite, "func", test_func);
}

