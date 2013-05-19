#include <cunit/CUnit.h>
#include <test_destination_nmea_serial.h>
#include <navcom/destination/nmea_serial.h>
#include <common/macros.h>
#include <string.h>

static const struct proc_desc_t * proc = &nmea_serial;

static void test_(void)
{
	CU_FAIL();
}

void register_suite_destination_nmea_serial(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("destination/nmea_serial", NULL, NULL);

	CU_add_test(suite, "?", test_);
}

