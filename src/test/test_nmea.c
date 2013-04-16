#include <cunit/CUnit.h>
#include <common/endian.h>
#include <nmea/nmea.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_int.h>
#include <nmea/nmea_fix.h>
#include <nmea/nmea_checksum.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
	GP = General Positioning System (GPS)

		RMA = recommended minimum navigation
		RMB = recommended minimum navigation
		RMC = recommended minimum sentence
		DTM = datum reference
		GGA = global positioning system fix data
		GSA = active satelites
		GSV = satellites in view

	HC = Heading Compass (magnetic compass)
	P  = proprietary (PGR : proprietary garmin)
*/
static const char * SENTENCES[] = {
	"$GPRMC,201034,A,4702.4040,N,00818.3281,E,0.0,328.4,260807,0.6,E,A*17",
	"$GPRMC,201124,A,4702.3947,N,00818.3372,E,0.3,328.4,260807,0.6,E,A*10",
	"$GPRMC,201126,A,4702.3944,N,00818.3381,E,0.0,328.4,260807,0.6,E,A*1E",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,29,08,15,075,35,09,40,277,00,12,20,212,00*75",
	"$GPGSV,3,2,10,15,82,225,20,17,24,120,44,18,28,302,00,22,06,330,00*77",
	"$GPGSV,3,3,10,27,45,280,00,28,41,053,00*7F",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1495,f,*11",
	"$PGRMM,WGS 84*06",
	"$HCHDG,45.8,,,0.6,E*16",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,24,08,15,075,35,09,40,277,00,12,20,212,00*78",
	"$GPGSV,3,2,10,15,82,225,19,17,24,120,44,18,28,302,00,22,06,330,00*7D",
	"$GPGSV,3,3,10,27,45,280,00,28,41,053,00*7F",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1494,f,*10",
	"$PGRMM,WGS 84*06",
	"$HCHDG,46.2,,,0.6,E*1F",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,23,08,15,075,35,09,40,277,00,12,20,212,00*7F",
	"$GPGSV,3,2,10,15,82,225,19,17,24,120,44,18,28,302,00,22,06,330,00*7D",
	"$GPGSV,3,3,10,27,45,280,00,28,41,053,00*7F",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1494,f,*10",
	"$PGRMM,WGS 84*06",
	"$HCHDG,46.5,,,0.6,E*18",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,23,08,15,075,35,09,40,277,00,12,20,212,00*7F",
	"$GPGSV,3,2,10,15,82,225,19,17,24,120,44,18,28,302,00,22,06,330,00*7D",
	"$GPGSV,3,3,10,27,45,280,00,28,41,053,00*7F",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1494,f,*10",
	"$PGRMM,WGS 84*06",
	"$HCHDG,46.6,,,0.6,E*1B",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,24,08,15,075,35,09,40,277,25,12,20,212,00*7F",
	"$GPGSV,3,2,10,15,82,225,18,17,24,120,44,18,28,302,00,22,06,330,00*7C",
	"$GPGSV,3,3,10,27,45,280,00,28,41,053,00*7F",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1493,f,*17",
	"$PGRMM,WGS 84*06",
	"$HCHDG,46.1,,,0.6,E*1C",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,24,08,15,075,35,09,40,277,25,12,20,212,00*7F",
	"$GPGSV,3,2,10,15,82,225,18,17,24,120,44,18,28,302,00,22,06,330,00*7C",
	"$GPGSV,3,3,10,27,46,281,00,28,41,053,00*7D",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1493,f,*17",
	"$PGRMM,WGS 84*06",
	"$HCHDG,47.4,,,0.6,E*18",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,24,08,15,075,35,09,40,277,25,12,20,212,00*7F",
	"$GPGSV,3,2,10,15,82,225,18,17,24,120,44,18,28,302,00,22,06,330,00*7C",
	"$GPGSV,3,3,10,27,46,281,00,28,41,053,23*7C",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1493,f,*17",
	"$PGRMM,WGS 84*06",
	"$HCHDG,48.7,,,0.6,E*14",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,25,08,15,075,35,09,40,277,25,12,20,212,00*7E",
	"$GPGSV,3,2,10,15,82,225,19,17,24,120,44,18,28,302,00,22,06,330,00*7D",
	"$GPGSV,3,3,10,27,46,281,00,28,39,053,23*73",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1495,f,*11",
	"$PGRMM,WGS 84*06",
	"$HCHDG,49.3,,,0.6,E*11",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,25,08,15,075,35,09,40,277,25,12,20,212,00*7E",
	"$GPGSV,3,2,10,15,82,225,19,17,24,120,44,18,28,302,00,22,06,330,00*7D",
	"$GPGSV,3,3,10,27,46,281,00,28,39,053,00*72",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1494,f,*10",
	"$PGRMM,WGS 84*06",
	"$HCHDG,50.2,,,0.6,E*18",
	"$GPRTE,1,1,c,*37",
	"$GPRMC,,V,,,,,,,300510,0.6,E,N*39",
	"$GPRMB,V,,,,,,,,,,,,V,N*04",
	"$GPGGA,,,,,,0,03,,,M,,M,,*65",
	"$GPGSA,A,1,05,08,,,,17,,,,,,,,,*15",
	"$GPGSV,3,1,10,05,07,188,25,08,15,075,35,09,40,277,25,12,20,212,00*7E",
	"$GPGSV,3,2,10,15,82,225,24,17,24,120,44,18,28,302,00,22,06,330,00*73",
	"$GPGSV,3,3,10,27,46,281,00,28,39,053,00*72",
	"$GPGLL,,,,,,V,N*64",
	"$GPBOD,,T,,M,,*47",
	"$GPVTG,,T,,M,,N,,K,N*2C",
	"$PGRME,,M,,M,,M*00",
	"$PGRMZ,1494,f,*10",
	"$PGRMM,WGS 84*06",
	"$HCHDG,50.3,,,0.6,E*19",
};

static void test_nmea_fix_check_zero(void)
{
	struct nmea_fix_t t;

	CU_ASSERT_EQUAL(nmea_fix_check_zero(NULL), -1);

	t.i = 0; t.d = 0;
	CU_ASSERT_EQUAL(nmea_fix_check_zero(&t), 0);

	t.i = 1; t.d = 0;
	CU_ASSERT_EQUAL(nmea_fix_check_zero(&t), -2);

	t.i = 0; t.d = 1;
	CU_ASSERT_EQUAL(nmea_fix_check_zero(&t), -2);
}

static void test_nmea_time_check_zero(void)
{
	struct nmea_time_t t;

	CU_ASSERT_EQUAL(nmea_time_check_zero(NULL), -1);

	t.h = 0; t.m = 0; t.s = 0; t.ms = 0;
	CU_ASSERT_EQUAL(nmea_time_check_zero(&t), 0);

	t.h = 1; t.m = 0; t.s = 0; t.ms = 0;
	CU_ASSERT_EQUAL(nmea_time_check_zero(&t), -2);

	t.h = 0; t.m = 1; t.s = 0; t.ms = 0;
	CU_ASSERT_EQUAL(nmea_time_check_zero(&t), -2);

	t.h = 0; t.m = 0; t.s = 1; t.ms = 0;
	CU_ASSERT_EQUAL(nmea_time_check_zero(&t), -2);

	t.h = 0; t.m = 0; t.s = 0; t.ms = 1;
	CU_ASSERT_EQUAL(nmea_time_check_zero(&t), -2);
}

static void test_check_date_zero(void)
{
	struct nmea_date_t t;

	CU_ASSERT_EQUAL(nmea_date_check_zero(NULL), -1);

	t.y = 0; t.m = 0; t.d = 0;
	CU_ASSERT_EQUAL(nmea_date_check_zero(&t), 0);

	t.y = 1; t.m = 0; t.d = 0;
	CU_ASSERT_EQUAL(nmea_date_check_zero(&t), -2);

	t.y = 0; t.m = 1; t.d = 0;
	CU_ASSERT_EQUAL(nmea_date_check_zero(&t), -2);

	t.y = 0; t.m = 0; t.d = 1;
	CU_ASSERT_EQUAL(nmea_date_check_zero(&t), -2);
}

static int test_parse_int(const char * s)
{
	uint32_t v;
	const char * p;

	p = parse_int(s, s+strlen(s), &v);
	return p == (s + strlen(s));
}

static void test_parsing_basic_int(void)
{
	CU_ASSERT_EQUAL(test_parse_int("0"),    1);
	CU_ASSERT_EQUAL(test_parse_int("10"),   1);
	CU_ASSERT_EQUAL(test_parse_int("00"),   1);
	CU_ASSERT_EQUAL(test_parse_int("0123"), 1);
	CU_ASSERT_EQUAL(test_parse_int("9999"), 1);
	CU_ASSERT_EQUAL(test_parse_int("10,"),  0);
	CU_ASSERT_EQUAL(test_parse_int(""),     1);
	CU_ASSERT_EQUAL(test_parse_int(","),    0);
	CU_ASSERT_EQUAL(test_parse_int("0.0"),  0);
}

static int test_nmea_fix_parse(const char * s)
{
	struct nmea_fix_t v;
	const char * p;

	p = nmea_fix_parse(s, s+strlen(s), &v);
	return p == (s + strlen(s));
}

static void test_parsing_nmea_fix(void)
{
	CU_ASSERT_EQUAL(test_nmea_fix_parse("123456789.1234567"), 1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("3.14159265365"),     1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("1.2"),               1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("3"),                 1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse(".5"),                1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("3."),                1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("1.0001"),            1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("1.1,"),              0);
	CU_ASSERT_EQUAL(test_nmea_fix_parse(""),                  1);
	CU_ASSERT_EQUAL(test_nmea_fix_parse("."),                 1);
}

static int test_nmea_date_parse(const char * s)
{
	struct nmea_date_t t;
	const char * e = s + strlen(s);

	return nmea_date_parse(s, e, &t) == e && !nmea_date_check(&t);
}

static void test_parsing_nmea_date(void)
{
	CU_ASSERT_EQUAL(test_nmea_date_parse("010100"),  1);
	CU_ASSERT_EQUAL(test_nmea_date_parse("999999"),  0);
	CU_ASSERT_EQUAL(test_nmea_date_parse("320100"),  0);
	CU_ASSERT_EQUAL(test_nmea_date_parse("000100"),  0);
	CU_ASSERT_EQUAL(test_nmea_date_parse("011300"),  0);
	CU_ASSERT_EQUAL(test_nmea_date_parse("311299"),  1);
	CU_ASSERT_EQUAL(test_nmea_date_parse("999999,"), 0);
	CU_ASSERT_EQUAL(test_nmea_date_parse("010100,"), 0);
}

static int test_nmea_time_parse(const char * s)
{
	struct nmea_time_t t;
	const char * e = s + strlen(s);

	return nmea_time_parse(s, e, &t) == e && !nmea_time_check(&t);
}

static void test_parsing_nmea_time(void)
{
	CU_ASSERT_EQUAL(test_nmea_time_parse("123456"),     1);
	CU_ASSERT_EQUAL(test_nmea_time_parse("123456,"),    0);
	CU_ASSERT_EQUAL(test_nmea_time_parse("123456.345"), 1);
	CU_ASSERT_EQUAL(test_nmea_time_parse("123456."),    1);
	CU_ASSERT_EQUAL(test_nmea_time_parse("0"),          1);
	CU_ASSERT_EQUAL(test_nmea_time_parse(""),           1);
}

static int test_parse_lat(const char * s)
{
	struct nmea_angle_t t;
	const char * e = s + strlen(s);

	return nmea_angle_parse(s, e, &t) == e && !nmea_check_latitude(&t);
}

static void test_parsing_nmea_lat(void)
{
	CU_ASSERT_EQUAL(test_parse_lat("1234.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lat("1234,0000"),  0);
	CU_ASSERT_EQUAL(test_parse_lat("1234.0000,"), 0);
	CU_ASSERT_EQUAL(test_parse_lat("9000.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lat("0000.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lat("8959.9999"),  1);
	CU_ASSERT_EQUAL(test_parse_lat("8960.0000"),  0);
	CU_ASSERT_EQUAL(test_parse_lat("9000.1000"),  0);
}

static int test_parse_lon(const char * s)
{
	struct nmea_angle_t t;
	const char * e = s + strlen(s);

	return nmea_angle_parse(s, e, &t) == e && !nmea_check_longitude(&t);
}

static void test_parsing_nmea_lon(void)
{
	CU_ASSERT_EQUAL(test_parse_lon("12345.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lon("01234,0000"),  0);
	CU_ASSERT_EQUAL(test_parse_lon("01234.0000,"), 0);
	CU_ASSERT_EQUAL(test_parse_lon("09000.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lon("00000.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lon("17959.9999"),  1);
	CU_ASSERT_EQUAL(test_parse_lon("17960.0000"),  0);
	CU_ASSERT_EQUAL(test_parse_lon("18000.0000"),  1);
	CU_ASSERT_EQUAL(test_parse_lon("18000.1000"),  0);
}

static void test_sentence_parsing()
{
	struct nmea_t info;
	unsigned int i;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		memset(&info, 0, sizeof(info));
		CU_ASSERT_EQUAL(nmea_read(&info, SENTENCES[i]), 0);
	}
}

static void test_basic_string_writing(void)
{
	enum { SIZE = 128 };

	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	CU_ASSERT_EQUAL(write_string(buf, SIZE, ""), 0);

	memset(buf, 0, sizeof(buf));
	CU_ASSERT_EQUAL(write_string(buf, 0, "hello"), -1);

	memset(buf, 0, sizeof(buf));
	CU_ASSERT_EQUAL(write_string(NULL, SIZE, "hello"), -1);

	memset(buf, 0, sizeof(buf));
	CU_ASSERT_EQUAL(write_string(buf, SIZE, "hello"), strlen("hello"));
}

static void test_nmea_fix_write(const struct nmea_fix_t * t, uint32_t ni, uint32_t nd, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	rc = nmea_fix_write(buf, SIZE, t, ni, nd);

	CU_ASSERT_EQUAL(rc, (int)strlen(outcome));
	CU_ASSERT_STRING_EQUAL(buf, outcome);
}

static void test_basic_fix_writing(void)
{
	struct nmea_fix_t t;

	t.i =     0; t.d =      0; test_nmea_fix_write(&t, 1, 6, "0.000000");
	t.i =     0; t.d =      1; test_nmea_fix_write(&t, 1, 6, "0.000001");
	t.i =     0; t.d =     10; test_nmea_fix_write(&t, 1, 6, "0.000010");
	t.i =     0; t.d =    100; test_nmea_fix_write(&t, 1, 6, "0.000100");
	t.i =     0; t.d =   1000; test_nmea_fix_write(&t, 1, 6, "0.001000");
	t.i =     0; t.d =  10000; test_nmea_fix_write(&t, 1, 6, "0.010000");
	t.i =     0; t.d = 100000; test_nmea_fix_write(&t, 1, 6, "0.100000");
	t.i =     1; t.d =      0; test_nmea_fix_write(&t, 5, 6, "    1.000000");
	t.i =    10; t.d =      0; test_nmea_fix_write(&t, 5, 6, "   10.000000");
	t.i =   100; t.d =      0; test_nmea_fix_write(&t, 5, 6, "  100.000000");
	t.i =  1000; t.d =      0; test_nmea_fix_write(&t, 5, 6, " 1000.000000");
	t.i = 10000; t.d =      0; test_nmea_fix_write(&t, 5, 6, "10000.000000");
}

static void test_nmea_time_write(const struct nmea_time_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	rc = nmea_time_write(buf, SIZE, t);

	CU_ASSERT_EQUAL(rc, (int)strlen(outcome));
	CU_ASSERT_STRING_EQUAL(buf, outcome);
}

static void test_basic_time_writing(void)
{
	struct nmea_time_t t;
	t.ms = 0;

	t.h =  0; t.m =  0; t.s =  0; test_nmea_time_write(&t, "000000");
	t.h = 10; t.m =  0; t.s =  0; test_nmea_time_write(&t, "100000");
	t.h = 23; t.m =  0; t.s =  0; test_nmea_time_write(&t, "230000");
	t.h = 24; t.m =  0; t.s =  0; test_nmea_time_write(&t, "");
	t.h =  0; t.m =  0; t.s =  0; test_nmea_time_write(&t, "000000");
	t.h = 10; t.m = 10; t.s =  0; test_nmea_time_write(&t, "101000");
	t.h = 23; t.m = 59; t.s =  0; test_nmea_time_write(&t, "235900");
	t.h = 24; t.m = 60; t.s =  0; test_nmea_time_write(&t, "");
	t.h =  0; t.m =  0; t.s =  0; test_nmea_time_write(&t, "000000");
	t.h = 10; t.m = 10; t.s = 10; test_nmea_time_write(&t, "101010");
	t.h = 23; t.m = 59; t.s = 59; test_nmea_time_write(&t, "235959");
	t.h = 24; t.m = 60; t.s = 60; test_nmea_time_write(&t, "");
}

static void test_nmea_date_write(const struct nmea_date_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	rc = nmea_date_write(buf, SIZE, t);

	CU_ASSERT_EQUAL(rc, (int)strlen(outcome));
	CU_ASSERT_STRING_EQUAL(buf, outcome);
}

static void test_basic_date_writing(void)
{
	struct nmea_date_t t;

	t.y =   0; t.m =  0; t.d =  0; test_nmea_date_write(&t, "");
	t.y =   0; t.m =  1; t.d =  1; test_nmea_date_write(&t, "010100");
	t.y =  99; t.m =  1; t.d =  1; test_nmea_date_write(&t, "010199");
	t.y = 100; t.m =  1; t.d =  1; test_nmea_date_write(&t, "010100");
	t.y =   0; t.m =  0; t.d =  1; test_nmea_date_write(&t, "");
	t.y =  99; t.m =  1; t.d =  1; test_nmea_date_write(&t, "010199");
	t.y = 100; t.m = 12; t.d =  1; test_nmea_date_write(&t, "011200");
	t.y =   0; t.m = 13; t.d =  1; test_nmea_date_write(&t, "");
	t.y =   0; t.m =  1; t.d =  1; test_nmea_date_write(&t, "010100");
	t.y =   0; t.m =  1; t.d = 31; test_nmea_date_write(&t, "310100");
	t.y =   0; t.m =  1; t.d = 32; test_nmea_date_write(&t, "");
}

static void test_write_lat(const struct nmea_angle_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	rc = nmea_write_latitude(buf, SIZE, t);

	CU_ASSERT_EQUAL(rc, (int)strlen(outcome));
	CU_ASSERT_STRING_EQUAL(buf, outcome);
}

static void test_basic_latitude_writing(void)
{
	struct nmea_angle_t t;
	t.d = 0;
	t.m = 0;
	t.s.i = 0;
	t.s.d = 0;

	t.d =  0; t.m =  0; t.s.i =  0; test_write_lat(&t, "0000.0000");
	t.d =  1; t.m =  0; t.s.i =  0; test_write_lat(&t, "0100.0000");
	t.d = 89; t.m =  0; t.s.i =  0; test_write_lat(&t, "8900.0000");
	t.d = 90; t.m =  0; t.s.i =  0; test_write_lat(&t, "9000.0000");
	t.d = 91; t.m =  0; t.s.i =  0; test_write_lat(&t, "");
	t.d =  0; t.m =  0; t.s.i =  0; test_write_lat(&t, "0000.0000");
	t.d =  1; t.m =  1; t.s.i =  0; test_write_lat(&t, "0101.0000");
	t.d = 89; t.m = 59; t.s.i =  0; test_write_lat(&t, "8959.0000");
	t.d = 90; t.m = 60; t.s.i =  0; test_write_lat(&t, "");
	t.d = 89; t.m = 61; t.s.i =  0; test_write_lat(&t, "");
	t.d =  0; t.m =  0; t.s.i =  0; test_write_lat(&t, "0000.0000");
	t.d =  1; t.m =  0; t.s.i =  1; test_write_lat(&t, "0100.0166");
	t.d = 89; t.m =  0; t.s.i = 59; test_write_lat(&t, "8900.9833");
	t.d = 90; t.m =  0; t.s.i = 60; test_write_lat(&t, "");
	t.d = 89; t.m =  0; t.s.i = 61; test_write_lat(&t, "");
}

static void test_write_lon(const struct nmea_angle_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	rc = nmea_write_lonitude(buf, SIZE, t);

	CU_ASSERT_EQUAL(rc, (int)strlen(outcome));
	CU_ASSERT_STRING_EQUAL(buf, outcome);
}

static void test_basic_longitude_writing(void)
{
	struct nmea_angle_t t;
	t.d = 0;
	t.m = 0;
	t.s.i = 0;
	t.s.d = 0;

	t.d =   0; t.m =  0; t.s.i =  0; test_write_lon(&t, "00000.0000");
	t.d =   1; t.m =  0; t.s.i =  0; test_write_lon(&t, "00100.0000");
	t.d = 179; t.m =  0; t.s.i =  0; test_write_lon(&t, "17900.0000");
	t.d = 180; t.m =  0; t.s.i =  0; test_write_lon(&t, "18000.0000");
	t.d = 181; t.m =  0; t.s.i =  0; test_write_lon(&t, "");
	t.d =   0; t.m =  1; t.s.i =  0; test_write_lon(&t, "00001.0000");
	t.d = 179; t.m = 59; t.s.i =  0; test_write_lon(&t, "17959.0000");
	t.d =   0; t.m = 60; t.s.i =  0; test_write_lon(&t, "");
	t.d = 179; t.m = 60; t.s.i =  0; test_write_lon(&t, "");
	t.d = 179; t.m = 61; t.s.i =  0; test_write_lon(&t, "");
	t.d =   1; t.m =  0; t.s.i =  1; test_write_lon(&t, "00100.0166");
	t.d = 179; t.m =  0; t.s.i = 59; test_write_lon(&t, "17900.9833");
	t.d =   0; t.m =  0; t.s.i = 60; test_write_lon(&t, "");
	t.d = 181; t.m =  0; t.s.i = 61; test_write_lon(&t, "");
}

static void test_sentence_writing(void)
{
	unsigned int i;
	int rc;
	char buf[128];
	struct nmea_t nmea;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		memset(buf, 0, sizeof(buf));
		memset(&nmea, 0, sizeof(nmea));
		rc = nmea_read(&nmea, SENTENCES[i]);
		CU_ASSERT_EQUAL(rc, 0);

		rc = nmea_write(buf, sizeof(buf), &nmea);
		if (rc == -3) {
			continue; /* writing not supported, which is ok */
		}
		if (rc == -2) {
			printf("nmea_write: unknown sentence: '%s' (raw:'%s', type:%u)\n", SENTENCES[i], nmea.raw, nmea.type);
			CU_ASSERT_NOT_EQUAL_FATAL(rc, -2);
		}
		CU_ASSERT_TRUE(rc >= 0);
		CU_ASSERT_STRING_EQUAL(buf, SENTENCES[i]);
	}
}

static void test_endianess(void)
{
	int rc;
	struct nmea_t a;
	struct nmea_t b;
	unsigned int i;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(SENTENCES[0]); ++i) {
		memset(&a, 0, sizeof(a));
		CU_ASSERT_EQUAL(nmea_read(&a, SENTENCES[i]), 0);
		memcpy(&b, &a, sizeof(b));
		rc = nmea_hton(&a);
		CU_ASSERT(rc != -1);
		CU_ASSERT(rc != -2);
		if (rc == -3) continue;
		rc = nmea_ntoh(&a);
		CU_ASSERT(rc != -1);
		CU_ASSERT(rc != -2);
		if (rc == -3) continue;
		CU_ASSERT_EQUAL(memcmp(&a, &b, sizeof(a)), 0);
	}
}

static void test_conv_float_fix(float expected, const struct nmea_fix_t * fix)
{
	int rc;
	float f;

	rc = nmea_fix_to_float(&f, fix);
	CU_ASSERT_EQUAL(rc, 0);
	CU_ASSERT_EQUAL(f, expected);
}

static void test_convert_nmea_fix_float(void)
{
	struct nmea_fix_t fix;
	float f;
	int rc;

	fix.i = 0;
	fix.d = 0;
	f = 0.0f;

	rc = nmea_fix_to_float(NULL, NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = nmea_fix_to_float(&f, NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = nmea_fix_to_float(NULL, &fix);
	CU_ASSERT_EQUAL(rc, -1);

	fix.i = 0; fix.d = 0;      test_conv_float_fix(0.0f, &fix);
	fix.i = 0; fix.d = 500000; test_conv_float_fix(0.5f, &fix);
	fix.i = 1; fix.d = 0;      test_conv_float_fix(1.0f, &fix);
	fix.i = 2; fix.d = 0;      test_conv_float_fix(2.0f, &fix);
	fix.i = 3; fix.d = 0;      test_conv_float_fix(3.0f, &fix);
}

static void test_conv_double_fix(double expected, const struct nmea_fix_t * fix)
{
	int rc;
	double d;

	rc = nmea_fix_to_double(&d, fix);
	CU_ASSERT_EQUAL(rc, 0);
	CU_ASSERT_EQUAL(d, expected);
}

static void test_convert_nmea_fix_double(void)
{
	struct nmea_fix_t fix;
	double d;
	int rc;

	fix.i = 0;
	fix.d = 0;
	d = 0.0;

	rc = nmea_fix_to_double(NULL, NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = nmea_fix_to_double(&d, NULL);
	CU_ASSERT_EQUAL(rc, -1);

	rc = nmea_fix_to_float(NULL, &fix);
	CU_ASSERT_EQUAL(rc, -1);

	fix.i = 0; fix.d = 0;      test_conv_double_fix(0.0, &fix);
	fix.i = 0; fix.d = 500000; test_conv_double_fix(0.5, &fix);
	fix.i = 1; fix.d = 0;      test_conv_double_fix(1.0, &fix);
	fix.i = 2; fix.d = 0;      test_conv_double_fix(2.0, &fix);
	fix.i = 3; fix.d = 0;      test_conv_double_fix(3.0, &fix);
}

static void test_checksum(void)
{
	struct checksum_test_t
	{
		const char * str;
		uint8_t chk;
	};

	static const struct checksum_test_t TESTS[] =
	{
		{ "GPRMC,201034,A,4702.4040,N,00818.3281,E,0.0,328.4,260807,0.6,E,A", 0x17 },
		{ "GPRMC,201124,A,4702.3947,N,00818.3372,E,0.3,328.4,260807,0.6,E,A", 0x10 },
		{ "GPRMC,201126,A,4702.3944,N,00818.3381,E,0.0,328.4,260807,0.6,E,A", 0x1E },
	};

	unsigned int i;
	uint8_t chk;

	for (i = 0 ; i < sizeof(TESTS) / sizeof(TESTS[0]); ++i) {
		chk = nmea_checksum(TESTS[i].str, TESTS[i].str + strlen(TESTS[i].str));
		CU_ASSERT_EQUAL(chk, TESTS[i].chk);
	}
}

static void test_checksum_check(void)
{
	struct checksum_test_t
	{
		const char * str;
	};

	static const struct checksum_test_t TESTS[] =
	{
		{ "$GPRMC,201034,A,4702.4040,N,00818.3281,E,0.0,328.4,260807,0.6,E,A*17" },
		{ "$GPRMC,201124,A,4702.3947,N,00818.3372,E,0.3,328.4,260807,0.6,E,A*10" },
		{ "$GPRMC,201126,A,4702.3944,N,00818.3381,E,0.0,328.4,260807,0.6,E,A*1E" },
	};

	unsigned int i;

	for (i = 0 ; i < sizeof(TESTS) / sizeof(TESTS[0]); ++i) {
		CU_ASSERT_EQUAL(nmea_checksum_check(TESTS[i].str, '$'), 0);
	}
}

static void test_checksum_write(void)
{
	struct checksum_test_t
	{
		const char * str;
		const char * chk;
	};

	static const struct checksum_test_t TESTS[] =
	{
		{ "GPRMC,201034,A,4702.4040,N,00818.3281,E,0.0,328.4,260807,0.6,E,A", "17" },
		{ "GPRMC,201124,A,4702.3947,N,00818.3372,E,0.3,328.4,260807,0.6,E,A", "10" },
		{ "GPRMC,201126,A,4702.3944,N,00818.3381,E,0.0,328.4,260807,0.6,E,A", "1E" },
	};

	unsigned int i;
	char buf[8];
	int rc;

	for (i = 0 ; i < sizeof(TESTS) / sizeof(TESTS[0]); ++i) {
		rc = nmea_checksum_write(buf, sizeof(buf),
			TESTS[i].str, TESTS[i].str + strlen(TESTS[i].str));
		CU_ASSERT_EQUAL(rc, 2);
		CU_ASSERT_STRING_EQUAL(buf, TESTS[i].chk);
	}
}

static void test_nmea_fix_endianess_hton()
{
	struct nmea_fix_t fix;

	fix.i = 0x12345678;
	fix.d = 0x87654321;

	nmea_fix_hton(&fix);
	if (endian_is_little()) {
		CU_ASSERT_EQUAL(fix.i, 0x78563412);
		CU_ASSERT_EQUAL(fix.d, 0x21436587);
	} else {
		CU_ASSERT_EQUAL(fix.i, 0x12345678);
		CU_ASSERT_EQUAL(fix.d, 0x87654321);
	}
}

static void test_nmea_fix_endianess_ntoh()
{
	struct nmea_fix_t fix;

	fix.i = 0x12345678;
	fix.d = 0x87654321;

	nmea_fix_ntoh(&fix);
	if (endian_is_little()) {
		CU_ASSERT_EQUAL(fix.i, 0x78563412);
		CU_ASSERT_EQUAL(fix.d, 0x21436587);
	} else {
		CU_ASSERT_EQUAL(fix.i, 0x12345678);
		CU_ASSERT_EQUAL(fix.d, 0x87654321);
	}
}

void register_suite_nmea(void)
{
	CU_Suite * suite;
	suite = CU_add_suite("nmea", NULL, NULL);
	CU_add_test(suite, "nmea fix: check zero", test_nmea_fix_check_zero);
	CU_add_test(suite, "nmea time: check zero", test_nmea_time_check_zero);
	CU_add_test(suite, "nmea date: check zero", test_check_date_zero);
	CU_add_test(suite, "parsing: basic int", test_parsing_basic_int);
	CU_add_test(suite, "parsing: nmea fix", test_parsing_nmea_fix);
	CU_add_test(suite, "parsing: nmea date", test_parsing_nmea_date);
	CU_add_test(suite, "parsing: nmea time", test_parsing_nmea_time);
	CU_add_test(suite, "parsing: nmea lat", test_parsing_nmea_lat);
	CU_add_test(suite, "parsing: nmea lon", test_parsing_nmea_lon);
	CU_add_test(suite, "parsing: sentences", test_sentence_parsing);
	CU_add_test(suite, "writing: string", test_basic_string_writing);
	CU_add_test(suite, "writing: nmea fix", test_basic_fix_writing);
	CU_add_test(suite, "writing: nmea time", test_basic_time_writing);
	CU_add_test(suite, "writing: nmea date", test_basic_date_writing);
	CU_add_test(suite, "writing: nmea lat", test_basic_latitude_writing);
	CU_add_test(suite, "writing: nmea lon", test_basic_longitude_writing);
	CU_add_test(suite, "writing: sentence", test_sentence_writing);
	CU_add_test(suite, "endianess", test_endianess);
	CU_add_test(suite, "conversion: float", test_convert_nmea_fix_float);
	CU_add_test(suite, "conversion: double", test_convert_nmea_fix_double);
	CU_add_test(suite, "nmea fix: endianess hton", test_nmea_fix_endianess_hton);
	CU_add_test(suite, "nmea fix: endianess ntoh", test_nmea_fix_endianess_ntoh);
	CU_add_test(suite, "checksum", test_checksum);
	CU_add_test(suite, "checksum check", test_checksum_check);
	CU_add_test(suite, "checksum write", test_checksum_write);
}

