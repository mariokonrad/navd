#include <cunit/CUnit.h>
#include <test_filterlist.h>
#include <navcom/filter_list.h>
#include <common/macros.h>
#include <stdlib.h>

static void test_init(void)
{
	struct filter_desc_list_t list;

	CU_ASSERT_EQUAL(filterlist_init(NULL), -1);
	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
	CU_ASSERT_EQUAL(list.data, NULL);
	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
}

static void test_free(void)
{
	struct filter_desc_list_t list;

	CU_ASSERT_EQUAL(filterlist_free(NULL), -1);

	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
	CU_ASSERT_EQUAL(filterlist_free(&list), 0);

	list.num = 1;
	list.data = (struct filter_desc_t *)malloc(sizeof(struct filter_desc_t));
	CU_ASSERT_EQUAL(filterlist_free(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
	CU_ASSERT_EQUAL(list.data, NULL);
}

static void test_append(void)
{
	struct filter_desc_list_t list;
	struct filter_desc_t desc;

	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
	CU_ASSERT_EQUAL(filterlist_append(NULL, NULL), -1);
	CU_ASSERT_EQUAL(filterlist_append(&list, NULL), -1);
	CU_ASSERT_EQUAL(filterlist_append(NULL, &desc), -1);
	CU_ASSERT_EQUAL(filterlist_append(&list, &desc), 0);
	CU_ASSERT_EQUAL(filterlist_free(&list), 0);

	memset(&desc, 0, sizeof(desc));

	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
	CU_ASSERT_EQUAL(filterlist_append(&list, &desc), 0);
	CU_ASSERT_EQUAL(list.num, 1);
	CU_ASSERT_EQUAL(filterlist_append(&list, &desc), 0);
	CU_ASSERT_EQUAL(list.num, 2);
	CU_ASSERT_EQUAL(filterlist_free(&list), 0);
}

static void test_find(void)
{
	struct filter_desc_list_t list;
	struct filter_desc_t desc;
	const struct filter_desc_t const * desc_new;

	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
	CU_ASSERT_EQUAL(filterlist_find(NULL, NULL), NULL);
	CU_ASSERT_EQUAL(filterlist_find(&list, NULL), NULL);
	CU_ASSERT_EQUAL(filterlist_find(NULL, ""), NULL);
	CU_ASSERT_EQUAL(filterlist_find(&list, ""), NULL);

	memset(&desc, 0, sizeof(desc));
	desc.name = "name";

	CU_ASSERT_EQUAL(filterlist_init(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
	CU_ASSERT_EQUAL(filterlist_append(&list, &desc), 0);
	CU_ASSERT_EQUAL(list.num, 1);

	CU_ASSERT_EQUAL(filterlist_find(&list, "test"), NULL);

	desc_new = filterlist_find(&list, "name");
	CU_ASSERT_NOT_EQUAL(desc_new, NULL);
	CU_ASSERT_STRING_EQUAL(desc_new->name, "name");

	CU_ASSERT_EQUAL(filterlist_free(&list), 0);
	CU_ASSERT_EQUAL(list.num, 0);
	CU_ASSERT_EQUAL(list.data, NULL);
}

void register_suite_filterlist(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("filterlist", NULL, NULL);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "free", test_free);
	CU_add_test(suite, "append", test_append);
	CU_add_test(suite, "find", test_find);
}

