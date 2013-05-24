#include <cunit/CUnit.h>
#include <test_source_src_lua.h>
#include <navcom/source/src_lua.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <lua/lua.h>

static const struct proc_desc_t * proc = &src_lua;
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

static void prepare_script(const char * code)
{
	int rc;

	rc = ftruncate(fd, 0);
	CU_ASSERT_EQUAL(rc, 0);

	rc = write(fd, code, strlen(code));
	CU_ASSERT_EQUAL(rc, (int)strlen(code));

	rc = lseek(fd, SEEK_SET, 0);
	CU_ASSERT_EQUAL(rc, 0);
}

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->exit);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_FAILURE);
}

/**
 * @todo Check for values after 'init'
 */
static void test_init(void)
{
	struct property_list_t properties;
	struct proc_config_t config;

	proc_config_init(&config);

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);

	proplist_init(&properties);
	proplist_set(&properties, "period", "zzz");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);
	proplist_free(&properties);

	proplist_init(&properties);
	proplist_set(&properties, "period", "1000");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);
	proplist_free(&properties);
}

static void test_init_noscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	int rc;

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", "/dev/null");
	proplist_set(&properties, "period", "1000");

	rc = proc->init(&config, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	rc = proc->exit(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_emptyscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	int rc;

	const char SCRIPT[] = "\n";

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);
	proplist_set(&properties, "period", "1000");

	prepare_script(SCRIPT);

	rc = proc->init(&config, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	rc = proc->exit(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_invalidscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	int rc;

	const char SCRIPT[] =
		"fucntion def()\n"
		"	local a = 1\n"
		"end\n";

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);
	proplist_set(&properties, "period", "1000");

	prepare_script(SCRIPT);

	rc = proc->init(&config, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	rc = proc->exit(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_scripterror(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	lua_State * lua;
	int rc;

	const char SCRIPT[] =
		"function abc(t)\n"
		"	return t.def + t.def\n"
		"end\n"
		"function def()\n"
		"	abc({})\n"
		"end\n";

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);
	proplist_set(&properties, "period", "1000");

	prepare_script(SCRIPT);

	rc = proc->init(&config, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, EXIT_SUCCESS);
	CU_ASSERT_PTR_NOT_NULL_FATAL(config.data);

	lua = (lua_State *)config.data;
	lua_getglobal(lua, "def");
	lua_pcall(lua, 0, 0, 0);

	rc = proc->exit(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_source_src_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/src_lua", setup, cleanup);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "exit", test_exit);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "init: no script", test_init_noscript);
	CU_add_test(suite, "init: empty script", test_init_emptyscript);
	CU_add_test(suite, "init: invalid script", test_init_invalidscript);
	CU_add_test(suite, "init: script error", test_init_scripterror);
}

