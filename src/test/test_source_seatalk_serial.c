#include <cunit/CUnit.h>
#include <test_source_seatalk_serial.h>
#include <navcom/source/seatalk_serial.h>
#include <navcom/source/seatalk_serial_private.h>
#include <common/macros.h>
#include <string.h>
#include <stdlib.h>

#if 0 /* TODO: disabled test data, to be used later */
static const uint8_t DATA[] =
{
	/* preliminary garbage */
	0x01,             /* bit=0 parity=0 : no error : ?    */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : ?    */
	0x01,             /* bit=0 parity=0 : no error : ?    */
	0x01,             /* bit=0 parity=0 : no error : ?    */
	0xff, 0xff,       /* bit=0 parity=1 : error    : ?    */
	0xff, 0xff,       /* bit=0 parity=1 : error    : ?    */
	0x01,             /* bit=0 parity=0 : no error : ?    */

	/* depth */
	0x00,             /* bit=1 parity=1 : no error : cmd  */
	0x02,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x60, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x65, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* speed through water */
	0xff, 0x00, 0x26, /* bit=1 parity=0 : error    : cmd  */
	0x04,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* water temperature */
	0x27,             /* bit=1 parity=1 : no error : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0x64,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* apparent wind speed */
	0x11,             /* bit=1 parity=1 : no error : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x06, /* bit=0 parity=1 : error    : data */
	0x01,             /* bit=0 parity=0 : no error : data */

	/* speed through water */
	0xff, 0x00, 0x20, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* water temperature */
	0xff, 0x00, 0x23, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x33, /* bit=0 parity=1 : error    : data */
	0x5b,             /* bit=0 parity=0 : no error : data */

	/* apparent wind angle */
	0xff, 0x00, 0x10, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x14, /* bit=0 parity=1 : error    : data */

	/* depth */
	0x00,             /* bit=1 parity=1 : no error : cmd  */
	0x02,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x60, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x65, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* depth, collision */
	0x00,             /* bit=1 parity=1 : no error : cmd  */
	0x02,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x60, /* bit=0 parity=1 : error    : data */

	/* apparent wind angle */
	0xff, 0x00, 0x10, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x14, /* bit=0 parity=1 : error    : data */
};
#endif

static const struct proc_desc_t * proc = &seatalk_serial;

static void test_existance(void)
{
	CU_ASSERT_PTR_NOT_NULL(proc);
	CU_ASSERT_PTR_NOT_NULL(proc->init);
	CU_ASSERT_PTR_NOT_NULL(proc->func);
	CU_ASSERT_PTR_NOT_NULL(proc->exit);
	CU_ASSERT_PTR_NOT_NULL(proc->help);
}

static void test_exit(void)
{
	CU_ASSERT_EQUAL(proc->exit(NULL), EXIT_FAILURE);
}

static void test_init(void)
{
	struct property_list_t properties;
	struct proc_config_t config;

	proc_config_init(&config);
	proplist_init(&properties);

	CU_ASSERT_EQUAL(proc->init(NULL, NULL), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(NULL, &properties), EXIT_FAILURE);
	CU_ASSERT_EQUAL(proc->init(&config, NULL), EXIT_FAILURE);

	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_PTR_NOT_NULL_FATAL(config.data);
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

static void test_init_name(void)
{
	struct property_list_t properties;
	struct proc_config_t config;
	struct seatalk_serial_data_t * data;

	proc_config_init(&config);
	proplist_init(&properties);

	proplist_set(&properties, "device", "/dev/null");
	CU_ASSERT_EQUAL(proc->init(&config, &properties), EXIT_SUCCESS);
	CU_ASSERT_PTR_NOT_NULL_FATAL(config.data);
	data = (struct seatalk_serial_data_t *)config.data;
	CU_ASSERT_STRING_EQUAL(data->serial_config.name, "/dev/null");
	CU_ASSERT_EQUAL(proc->exit(&config), EXIT_SUCCESS);

	proplist_free(&properties);
}

void register_suite_source_seatalk_serial(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("source/seatalk_serial", NULL, NULL);

	CU_add_test(suite, "existance", test_existance);
	CU_add_test(suite, "exit", test_exit);
	CU_add_test(suite, "init", test_init);
	CU_add_test(suite, "init: name", test_init_name);
}

