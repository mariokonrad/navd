#include <cunit/CUnit.h>
#include <test_device_simulator_serial_gps.h>
#include <device/simulator_serial_gps.h>
#include <common/macros.h>
#include <string.h>

static const struct device_operations_t * device = &simulator_serial_gps_operations;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(device);
	CU_ASSERT_PTR_NOT_NULL(device->open);
	CU_ASSERT_PTR_NOT_NULL(device->close);
	CU_ASSERT_PTR_NOT_NULL(device->read);
	CU_ASSERT_PTR_NOT_NULL(device->write);
}

static void test_open(void)
{
	struct device_t dev;

	device_init(&dev);

	CU_ASSERT_EQUAL(device->open(NULL, NULL), -1);

	dev.fd = -1;
	CU_ASSERT_EQUAL(device->open(&dev, NULL), 0);
	CU_ASSERT_EQUAL(dev.fd, 0);

	dev.fd = 1;
	CU_ASSERT_EQUAL(device->open(&dev, NULL), 0);
}

static void test_close(void)
{
	struct device_t dev;

	device_init(&dev);

	CU_ASSERT_EQUAL(device->close(NULL), -1);

	dev.fd = -1;
	CU_ASSERT_EQUAL(device->close(&dev), 0);

	dev.fd = 0;
	CU_ASSERT_EQUAL(device->close(&dev), 0);
	CU_ASSERT_EQUAL(dev.fd, -1);
}

static void test_open_close(void)
{
	struct device_t dev;

	device_init(&dev);
	CU_ASSERT_EQUAL(dev.fd, -1);
	CU_ASSERT_PTR_NULL(dev.data);
	CU_ASSERT_EQUAL(device->open(&dev, NULL), 0);
	CU_ASSERT_NOT_EQUAL(dev.fd, -1);
	CU_ASSERT_PTR_NOT_NULL(dev.data);
	CU_ASSERT_EQUAL(device->close(&dev), 0);
	CU_ASSERT_EQUAL(dev.fd, -1);
	CU_ASSERT_PTR_NULL(dev.data);
}

static void test_write(void)
{
	CU_ASSERT_EQUAL(device->write(NULL, NULL, 0), -1);
}

static void test_read(void)
{
	struct device_t dev;
	char buf[16];

	device_init(&dev);

	CU_ASSERT_EQUAL(device->read(NULL, NULL, 0), -1);
	CU_ASSERT_EQUAL(device->read(NULL, NULL, sizeof(buf)), -1);
	CU_ASSERT_EQUAL(device->read(NULL, buf, 0), -1);
	CU_ASSERT_EQUAL(device->read(NULL, buf, sizeof(buf)), -1);

	CU_ASSERT_EQUAL(device->open(&dev, NULL), 0);

	CU_ASSERT_EQUAL(device->read(&dev, NULL, 0), -1);
	CU_ASSERT_EQUAL(device->read(&dev, NULL, sizeof(buf)), -1);
	CU_ASSERT_EQUAL(device->read(&dev, buf, 0), 0);

	CU_ASSERT_EQUAL(device->read(&dev, buf, sizeof(buf)), sizeof(buf));

	dev.fd = -1;
	CU_ASSERT_EQUAL(device->read(&dev, buf, sizeof(buf)), -1);
}

void register_suite_device_simulator_serial_gps(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("device/simulator_serial_gps", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "open", test_open);
	CU_add_test(suite, "close", test_close);
	CU_add_test(suite, "open/close", test_open_close);
	CU_add_test(suite, "write", test_write);
	CU_add_test(suite, "read", test_read);
}

