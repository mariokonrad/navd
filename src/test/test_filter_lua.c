#include <cunit/CUnit.h>
#include <test_filter_lua.h>
#include <navcom/filter/filter_lua.h>
#include <common/macros.h>
#include <unistd.h>
#include <limits.h>

static const struct filter_desc_t * filter = &filter_lua;
static char tmpfilename[PATH_MAX];
static int fd = -1;

static int setup(void)
{
	strncpy(tmpfilename, "/tmp/test_filter_luaXXXXXXXX", sizeof(tmpfilename));
	fd = mkstemp(tmpfilename);
	return fd < 0;
}

static int cleanup(void)
{
	close(fd);
	unlink(tmpfilename);
	return 0;
}

static void test_configure(void)
{
	CU_ASSERT_NOT_EQUAL(filter->configure, NULL);
}

static void test_free_ctx(void)
{
	CU_ASSERT_NOT_EQUAL(filter->free_ctx, NULL);
}

static void test_configure_free(void)
{
	struct filter_context_t ctx;
	struct property_list_t properties;
	int rc;

	const char SCRIPT[] = "\n";

	proplist_init(&properties);

	CU_ASSERT_EQUAL(filter->configure(NULL, NULL), FILTER_FAILURE);
	CU_ASSERT_EQUAL(filter->configure(NULL, &properties), FILTER_FAILURE);
	CU_ASSERT_EQUAL(filter->configure(&ctx, &properties), FILTER_FAILURE);
	CU_ASSERT_EQUAL(filter->configure(&ctx, NULL), FILTER_FAILURE);

	proplist_set(&properties, "script", "");

	CU_ASSERT_EQUAL(filter->configure(&ctx, &properties), FILTER_FAILURE);

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);
	rc = write(fd, SCRIPT, strlen(SCRIPT));
	CU_ASSERT(rc == strlen(SCRIPT));

	proplist_set(&properties, "script", tmpfilename);
	CU_ASSERT_EQUAL(filter->configure(&ctx, &properties), FILTER_SUCCESS);
	CU_ASSERT_NOT_EQUAL(ctx.data, NULL);

	CU_ASSERT_EQUAL(filter->free_ctx(&ctx), FILTER_SUCCESS);
	proplist_free(&properties);
}

void register_suite_filter_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite(filter->name, setup, cleanup);

	CU_add_test(suite, "configure", test_configure);
	CU_add_test(suite, "free_ctx", test_free_ctx);
//	CU_add_test(suite, "configure / free", test_configure_free);
}

