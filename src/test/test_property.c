#include <cunit/CUnit.h>
#include <test_property.h>
#include <stdlib.h>
#include <common/property.h>
#include <common/macros.h>

static void test_list_init(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_init(NULL), -1);
	CU_ASSERT_EQUAL(proplist_init(&list), 0);
}

static void test_list_free(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_free(NULL), -1);

	CU_ASSERT_EQUAL(proplist_init(&list), 0);
	CU_ASSERT_EQUAL(proplist_free(&list), 0);
}

static void test_list_append(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_init(&list), 0);
	CU_ASSERT_EQUAL(proplist_append(NULL, NULL, NULL), -1);
	CU_ASSERT_EQUAL(proplist_append(NULL, NULL, "value"), -1);
	CU_ASSERT_EQUAL(proplist_append(NULL, "key", NULL), -1);
	CU_ASSERT_EQUAL(proplist_append(NULL, "key", "value"), -1);
	CU_ASSERT_EQUAL(proplist_append(&list, NULL, NULL), -1);
	CU_ASSERT_EQUAL(proplist_append(&list, NULL, "value"), -1);
	CU_ASSERT_EQUAL(proplist_append(&list, "key0", NULL), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key1", "value"), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key1", "value1"), 0);
	CU_ASSERT_EQUAL(proplist_free(&list), 0);
}

static void test_list_set(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_init(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
	CU_ASSERT_EQUAL(proplist_set(&list, "key", "value"), 0);
	CU_ASSERT_EQUAL(list.num, 1);
	CU_ASSERT_STRING_EQUAL(proplist_value(&list, "key"), "value");
	CU_ASSERT_EQUAL(proplist_set(&list, "key", "value1"), 1);
	CU_ASSERT_EQUAL(list.num, 1);
	CU_ASSERT_STRING_EQUAL(proplist_value(&list, "key"), "value1");
	CU_ASSERT_EQUAL(proplist_set(&list, "key1", NULL), 0);
	CU_ASSERT_EQUAL(list.num, 2);
	CU_ASSERT_EQUAL(proplist_value(&list, "key1"), NULL);
	CU_ASSERT_EQUAL(proplist_set(&list, "key1", "value2"), 1);
	CU_ASSERT_EQUAL(list.num, 2);
	CU_ASSERT_STRING_EQUAL(proplist_value(&list, "key1"), "value2");
	CU_ASSERT_EQUAL(proplist_free(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
}

static void test_list_contains(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_init(&list), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key0", NULL), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key1", "value"), 0);
	CU_ASSERT_EQUAL(proplist_contains(NULL, NULL), 0);
	CU_ASSERT_EQUAL(proplist_contains(NULL, ""), 0);
	CU_ASSERT_EQUAL(proplist_contains(NULL, "key0"), 0);
	CU_ASSERT_EQUAL(proplist_contains(&list, NULL), 0);
	CU_ASSERT_EQUAL(proplist_contains(&list, ""), 0);
	CU_ASSERT_NOT_EQUAL(proplist_contains(&list, "key0"), 0);
	CU_ASSERT_NOT_EQUAL(proplist_contains(&list, "key1"), 0);
	CU_ASSERT_EQUAL(proplist_free(&list), 0);
}

static void test_list_value(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_init(&list), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key0", NULL), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key1", "value"), 0);
	CU_ASSERT_EQUAL(proplist_value(NULL, NULL), NULL);
	CU_ASSERT_EQUAL(proplist_value(NULL, ""), NULL);
	CU_ASSERT_EQUAL(proplist_value(NULL, "key0"), NULL);
	CU_ASSERT_EQUAL(proplist_value(&list, NULL), NULL);
	CU_ASSERT_EQUAL(proplist_value(&list, ""), NULL);
	CU_ASSERT_EQUAL(proplist_value(&list, "key0"), NULL);
	CU_ASSERT_NOT_EQUAL(proplist_value(&list, "key1"), NULL);
	CU_ASSERT_STRING_EQUAL(proplist_value(&list, "key1"), "value");
	CU_ASSERT_EQUAL(proplist_free(&list), 0);
}

static void test_list_find(void)
{
	struct property_list_t list;

	CU_ASSERT_EQUAL(proplist_init(&list), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key0", NULL), 0);
	CU_ASSERT_EQUAL(proplist_append(&list, "key1", "value"), 0);
	CU_ASSERT_EQUAL(proplist_find(NULL, NULL), NULL);
	CU_ASSERT_EQUAL(proplist_find(NULL, ""), NULL);
	CU_ASSERT_EQUAL(proplist_find(NULL, "key0"), NULL);
	CU_ASSERT_EQUAL(proplist_find(&list, NULL), NULL);
	CU_ASSERT_EQUAL(proplist_find(&list, ""), NULL);
	CU_ASSERT_EQUAL(proplist_set(&list, "key2", "value2"), 0);
	CU_ASSERT_NOT_EQUAL(proplist_find(&list, "key2"), NULL);
	CU_ASSERT_NOT_EQUAL(proplist_find(&list, "key0"), NULL);
	CU_ASSERT_NOT_EQUAL(proplist_find(&list, "key1"), NULL);
	CU_ASSERT_EQUAL(proplist_free(&list), 0);
}

void register_suite_property(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("property", NULL, NULL);
	CU_add_test(suite, "list init", test_list_init);
	CU_add_test(suite, "list free", test_list_free);
	CU_add_test(suite, "list append", test_list_append);
	CU_add_test(suite, "list set", test_list_set);
	CU_add_test(suite, "list contains", test_list_contains);
	CU_add_test(suite, "list value", test_list_value);
	CU_add_test(suite, "list find", test_list_find);
}

