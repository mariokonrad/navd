#include <cunit/CUnit.h>
#include <test_destination_dst_lua.h>
#include <navcom/destination/dst_lua.h>
#include <navcom/destination/dst_lua_private.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <lua/lua.h>

static const struct proc_desc_t * proc = &dst_lua;
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
	CU_ASSERT_EQUAL(ftruncate(fd, 0), 0);
	CU_ASSERT_EQUAL(write(fd, code, strlen(code)), (int)strlen(code));
	CU_ASSERT_EQUAL(lseek(fd, SEEK_SET, 0), 0);
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

static void test_init_noscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", "/dev/null");

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(&config, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_emptyscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;

	const char SCRIPT[] = "\n";

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);

	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_invalidscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;

	const char SCRIPT[] =
		"fucntion def()\n"
		"	local a = 1\n"
		"end\n";

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_scripterror(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	struct dst_lua_data_t * data;

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

	prepare_script(SCRIPT);

	CU_ASSERT_EQUAL_FATAL(proc->init(&config, &properties), EXIT_SUCCESS);

	data = (struct dst_lua_data_t *)config.data;
	CU_ASSERT_PTR_NOT_NULL_FATAL(data);
	CU_ASSERT_PTR_NOT_NULL_FATAL(data->lua);

	lua_getglobal(data->lua, "def");
	lua_pcall(data->lua, 0, 0, 0);

	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_destination_dst_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/dst_lua", setup, cleanup);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "exit", test_exit);
	CU_add_test(suite, "init: no script", test_init_noscript);
	CU_add_test(suite, "init: empty script", test_init_emptyscript);
	CU_add_test(suite, "init: invalid script", test_init_invalidscript);
	CU_add_test(suite, "init: script error", test_init_scripterror);
}

