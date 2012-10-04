#include <cunit/CUnit.h>
#include <test_property_serial.h>
#include <stdlib.h>
#include <navcom/property_serial.h>
#include <common/macros.h>

static void test_read_device(void)
{
	struct property_list_t list;

	struct serial_config_t cfg = {
		"/dev/ttyUSB0",
		BAUD_4800,
		DATA_BIT_8,
		STOP_BIT_1,
		PARITY_NONE
	};

	proplist_init(&list);

	CU_ASSERT_EQUAL(prop_serial_read_device(NULL, NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_device(NULL, NULL, ""), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_device(NULL, &list, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_device(NULL, &list, ""), EXIT_FAILURE);

	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, NULL, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, NULL, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, &list, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, &list, "device"), EXIT_SUCCESS);

	proplist_set(&list, "device", "/dev/ttyUSB1");

	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_device(&cfg, &list, "device"), EXIT_SUCCESS);
	CU_ASSERT_STRING_EQUAL(cfg.name, "/dev/ttyUSB1");

	proplist_free(&list);
}

static void test_read_baudrate(void)
{
	struct property_list_t list;

	struct serial_config_t cfg = {
		"/dev/ttyUSB0",
		BAUD_4800,
		DATA_BIT_8,
		STOP_BIT_1,
		PARITY_NONE
	};

	proplist_init(&list);

	CU_ASSERT_EQUAL(prop_serial_read_baudrate(NULL, NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(NULL, NULL, ""), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(NULL, &list, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(NULL, &list, ""), EXIT_FAILURE);

	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, NULL, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, NULL, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, &list, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, &list, "baud"), EXIT_SUCCESS);

	proplist_set(&list, "baud", "9600");

	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.baud_rate, BAUD_4800);

	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, &list, "baud"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.baud_rate, BAUD_9600);

	proplist_set(&list, "baud", "12345");

	CU_ASSERT_EQUAL(prop_serial_read_baudrate(&cfg, &list, "baud"), EXIT_FAILURE);
	CU_ASSERT_EQUAL(cfg.baud_rate, BAUD_9600);

	proplist_free(&list);
}

static void test_read_parity(void)
{
	struct property_list_t list;

	struct serial_config_t cfg = {
		"/dev/ttyUSB0",
		BAUD_4800,
		DATA_BIT_8,
		STOP_BIT_1,
		PARITY_NONE
	};

	proplist_init(&list);

	CU_ASSERT_EQUAL(prop_serial_read_parity(NULL, NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_parity(NULL, NULL, ""), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_parity(NULL, &list, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_parity(NULL, &list, ""), EXIT_FAILURE);

	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, NULL, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, NULL, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, "parity"), EXIT_SUCCESS);

	proplist_set(&list, "parity", "none");

	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.parity, PARITY_NONE);

	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, "parity"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.parity, PARITY_NONE);

	proplist_set(&list, "parity", "even");

	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, "parity"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.parity, PARITY_EVEN);

	proplist_set(&list, "parity", "odd");

	CU_ASSERT_EQUAL(prop_serial_read_parity(&cfg, &list, "parity"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.parity, PARITY_ODD);

	proplist_free(&list);
}

static void test_read_databits(void)
{
	struct property_list_t list;

	struct serial_config_t cfg = {
		"/dev/ttyUSB0",
		BAUD_4800,
		DATA_BIT_8,
		STOP_BIT_1,
		PARITY_NONE
	};

	proplist_init(&list);

	CU_ASSERT_EQUAL(prop_serial_read_databits(NULL, NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_databits(NULL, NULL, ""), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_databits(NULL, &list, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_databits(NULL, &list, ""), EXIT_FAILURE);

	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, NULL, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, NULL, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, &list, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, &list, "data"), EXIT_SUCCESS);

	proplist_set(&list, "data", "7");

	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.data_bits, DATA_BIT_8);

	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, &list, "data"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.data_bits, DATA_BIT_7);

	proplist_set(&list, "data", "8");

	CU_ASSERT_EQUAL(prop_serial_read_databits(&cfg, &list, "data"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.data_bits, DATA_BIT_8);

	proplist_free(&list);
}

static void test_read_stopbits(void)
{
	struct property_list_t list;

	struct serial_config_t cfg = {
		"/dev/ttyUSB0",
		BAUD_4800,
		DATA_BIT_8,
		STOP_BIT_1,
		PARITY_NONE
	};

	proplist_init(&list);

	CU_ASSERT_EQUAL(prop_serial_read_stopbits(NULL, NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(NULL, NULL, ""), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(NULL, &list, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(NULL, &list, ""), EXIT_FAILURE);

	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, NULL, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, NULL, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, &list, NULL), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, &list, "stop"), EXIT_SUCCESS);

	proplist_set(&list, "stop", "2");

	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, &list, ""), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.stop_bits, STOP_BIT_1);

	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, &list, "stop"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.stop_bits, STOP_BIT_2);

	proplist_set(&list, "stop", "1");

	CU_ASSERT_EQUAL(prop_serial_read_stopbits(&cfg, &list, "stop"), EXIT_SUCCESS);
	CU_ASSERT_EQUAL(cfg.stop_bits, STOP_BIT_1);

	proplist_free(&list);
}

void register_suite_property_serial(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("property_serial", NULL, NULL);
	CU_add_test(suite, "read device", test_read_device);
	CU_add_test(suite, "read baudrate", test_read_baudrate);
	CU_add_test(suite, "read parity", test_read_parity);
	CU_add_test(suite, "read databits", test_read_databits);
	CU_add_test(suite, "read stopbits", test_read_stopbits);
}

