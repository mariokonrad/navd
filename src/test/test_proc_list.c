#include <cunit/CUnit.h>
#include <test_proc_list.h>
#include <navcom/proc_list.h>
#include <common/macros.h>
#include <string.h>

static void test_init(void)
{
	struct proc_desc_list_t pdl;
	int rc;

	rc = pdlist_init(NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = pdlist_init(&pdl);
	CU_ASSERT_EQUAL(rc, 0);

	CU_ASSERT_EQUAL(pdl.num, 0);
	CU_ASSERT_EQUAL(pdl.data, NULL);
}

static void test_init_free(void)
{
	struct proc_desc_list_t pdl;
	int rc;

	rc = pdlist_free(NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = pdlist_init(&pdl);
	CU_ASSERT_EQUAL(rc, 0);

	rc = pdlist_free(&pdl);
	CU_ASSERT_EQUAL(rc, 0);

	CU_ASSERT_EQUAL(pdl.num, 0);
	CU_ASSERT_EQUAL(pdl.data, NULL);
}

static void test_append(void)
{
	struct proc_desc_list_t pdl;
	struct proc_desc_t desc;
	int rc;

	memset(&desc, 0, sizeof(desc));
	desc.name = "foobar";

	rc = pdlist_init(&pdl);
	CU_ASSERT_EQUAL(rc, 0);

	rc = pdlist_append(NULL, NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = pdlist_append(&pdl, NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = pdlist_append(NULL, &desc);
	CU_ASSERT_EQUAL(rc, -1);

	CU_ASSERT_EQUAL(pdl.num, 0);
	rc = pdlist_append(&pdl, &desc);
	CU_ASSERT_EQUAL(rc, 0);
	CU_ASSERT_EQUAL(pdl.num, 1);

	rc = pdlist_free(&pdl);
	CU_ASSERT_EQUAL(rc, 0);
}

static void test_find(void)
{
	struct proc_desc_list_t pdl;
	struct proc_desc_t desc;
	int rc;
	struct proc_desc_t const * tmp;

	memset(&desc, 0, sizeof(desc));
	desc.name = "foobar";

	rc = pdlist_init(&pdl);
	CU_ASSERT_EQUAL(rc, 0);

	tmp = pdlist_find(NULL, NULL);
	CU_ASSERT_EQUAL(tmp, NULL);

	tmp = pdlist_find(NULL, "foobar");
	CU_ASSERT_EQUAL(tmp, NULL);

	tmp = pdlist_find(&pdl, NULL);
	CU_ASSERT_EQUAL(tmp, NULL);

	tmp = pdlist_find(&pdl, "foobar");
	CU_ASSERT_EQUAL(tmp, NULL);

	rc = pdlist_append(&pdl, &desc);
	CU_ASSERT_EQUAL(rc, 0);

	tmp = pdlist_find(&pdl, "foobar");
	CU_ASSERT_NOT_EQUAL(tmp, NULL);

	tmp = pdlist_find(&pdl, "teststring");
	CU_ASSERT_EQUAL(tmp, NULL);

	rc = pdlist_free(&pdl);
	CU_ASSERT_EQUAL(rc, 0);
}

void register_suite_proc_list(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("proc_list", NULL, NULL);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "init/free", test_init_free);
	CU_add_test(suite, "append", test_append);
	CU_add_test(suite, "find", test_find);
}

