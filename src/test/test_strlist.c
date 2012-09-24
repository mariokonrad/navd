#include <cunit/CUnit.h>
#include <test_strlist.h>
#include <stdlib.h>
#include <common/stringlist.h>
#include <common/macros.h>

static void test_init(void)
{
	struct string_list_t s;

	CU_ASSERT_EQUAL(strlist_init(&s), 0);
	CU_ASSERT_EQUAL(strlist_init(NULL), -1);
}

static void test_free(void)
{
	struct string_list_t s;

	CU_ASSERT_EQUAL(strlist_free(NULL), -1);

	strlist_init(&s);
	CU_ASSERT_EQUAL(strlist_free(&s), 0);
}

static void test_append(void)
{
	struct string_list_t s;

	strlist_init(&s);

	CU_ASSERT_EQUAL(strlist_append(NULL, "abc"), -1);
	CU_ASSERT_EQUAL(strlist_append(NULL, NULL), -1);
	CU_ASSERT_EQUAL(strlist_append(&s, NULL), -1);

	CU_ASSERT_EQUAL(strlist_append(&s, "a"), 0);
	CU_ASSERT_EQUAL(strlist_append(&s, "b"), 0);
	CU_ASSERT_EQUAL(strlist_append(&s, "c"), 0);

	CU_ASSERT_EQUAL(s.num, 3);

	CU_ASSERT_EQUAL(strlist_free(&s), 0);
	CU_ASSERT_EQUAL(s.num, 0);
	CU_ASSERT_EQUAL(s.data, NULL);
}

static void test_find(void)
{
	struct string_list_t s;

	strlist_init(&s);

	CU_ASSERT_EQUAL(strlist_find(NULL, NULL), -1);
	CU_ASSERT_EQUAL(strlist_find(NULL, "a"), -1);
	CU_ASSERT_EQUAL(strlist_find(&s, NULL), -1);

	CU_ASSERT_EQUAL(strlist_find(&s, "a"), 0);

	strlist_append(&s, "a");
	strlist_append(&s, "b");
	strlist_append(&s, "c");

	CU_ASSERT_EQUAL(strlist_find(&s, "a"), 1);
	CU_ASSERT_EQUAL(strlist_find(&s, "b"), 1);
	CU_ASSERT_EQUAL(strlist_find(&s, "c"), 1);

	strlist_free(&s);

	CU_ASSERT_EQUAL(strlist_find(&s, "a"), 0);
	CU_ASSERT_EQUAL(strlist_find(&s, "b"), 0);
	CU_ASSERT_EQUAL(strlist_find(&s, "c"), 0);
}

void register_suite_strlist(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("strlist", NULL, NULL);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "free", test_free);
	CU_add_test(suite, "append", test_append);
	CU_add_test(suite, "find", test_find);
}

