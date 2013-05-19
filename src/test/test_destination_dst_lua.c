#include <cunit/CUnit.h>
#include <test_destination_dst_lua.h>
#include <navcom/destination/dst_lua.h>
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
	CU_ASSERT_PTR_NOT_NULL(proc->configure);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->clean);
}

static void test_configure_noscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	int rc;

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", "/dev/null");

	rc = proc->configure(&config, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	rc = proc->clean(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_configure_emptyscript(void)
{
	struct proc_config_t config;
	struct property_list_t properties;
	int rc;

	const char SCRIPT[] = "\n";

	proc_config_init(&config);

	proplist_init(&properties);
	proplist_set(&properties, "script", tmpfilename);

	prepare_script(SCRIPT);

	rc = proc->configure(&config, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	rc = proc->clean(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_configure_invalidscript(void)
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

	prepare_script(SCRIPT);

	rc = proc->configure(&config, &properties);
	CU_ASSERT_EQUAL(rc, EXIT_FAILURE);

	rc = proc->clean(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_configure_scripterror(void)
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

	prepare_script(SCRIPT);

	rc = proc->configure(&config, &properties);
	CU_ASSERT_EQUAL_FATAL(rc, EXIT_SUCCESS);
	CU_ASSERT_PTR_NOT_NULL_FATAL(config.data);

	lua = (lua_State *)config.data;
	lua_getglobal(lua, "def");
	lua_pcall(lua, 0, 0, 0);

	rc = proc->clean(&config);
	CU_ASSERT_EQUAL(rc, EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_destination_dst_lua(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/dst_lua", setup, cleanup);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "configure: no script", test_configure_noscript);
	CU_add_test(suite, "configure: empty script", test_configure_emptyscript);
	CU_add_test(suite, "configure: invalid script", test_configure_invalidscript);
	CU_add_test(suite, "configure: script error", test_configure_scripterror);
}

