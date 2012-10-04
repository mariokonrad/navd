#include <cunit/CUnit.h>
#include <test_fileutil.h>
#include <common/fileutil.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

static char tmpfilename[PATH_MAX];
static int fd = -1;

static int setup(void)
{
	strncpy(tmpfilename, "/tmp/test_fileutilXXXXXXXX", sizeof(tmpfilename));
	fd = mkstemp(tmpfilename);
	return fd < 0;
}

static int cleanup(void)
{
	close(fd);
	unlink(tmpfilename);
	return 0;
}

static void test_readable(void)
{
	CU_ASSERT_EQUAL(file_is_readable(NULL), 0);
	CU_ASSERT_EQUAL(file_is_readable(""), 0);
	CU_ASSERT_EQUAL(file_is_readable("/tmp/test_fileutil-abc"), 0);
	CU_ASSERT_EQUAL(file_is_readable(tmpfilename), 1);
}

static void test_writable(void)
{
	CU_ASSERT_EQUAL(file_is_writable(NULL), 0);
	CU_ASSERT_EQUAL(file_is_writable(""), 0);
	CU_ASSERT_EQUAL(file_is_writable("/tmp/test_fileutil-abc"), 0);
	CU_ASSERT_EQUAL(file_is_writable(tmpfilename), 1);
}

void register_suite_fileutil(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("fileutil", setup, cleanup);
	CU_add_test(suite, "readable", test_readable);
	CU_add_test(suite, "wriable", test_writable);
}

