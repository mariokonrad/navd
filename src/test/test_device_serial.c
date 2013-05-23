#include <cunit/CUnit.h>
#include <test_device_serial.h>
#include <device/serial.h>
#include <common/macros.h>
#include <string.h>

static const struct device_operations_t * device = &serial_device_operations;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(device);
	CU_ASSERT_PTR_NOT_NULL(device->open);
	CU_ASSERT_PTR_NOT_NULL(device->close);
	CU_ASSERT_PTR_NOT_NULL(device->read);
	CU_ASSERT_PTR_NOT_NULL(device->write);
}

void register_suite_device_serial(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("device/serial", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
}

