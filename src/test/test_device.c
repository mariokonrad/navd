#include <cunit/CUnit.h>
#include <test_device.h>
#include <device/device.h>
#include <common/macros.h>

static void test_init(void)
{
	struct device_t dev;

	device_init(NULL);
	device_init(&dev);

	CU_ASSERT_EQUAL(dev.fd, -1);
	CU_ASSERT_EQUAL(dev.data, NULL);
}

void register_suite_device(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("device", NULL, NULL);
	CU_add_test(suite, "init", test_init);
}

