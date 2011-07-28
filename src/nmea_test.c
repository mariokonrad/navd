#include <nmea/nmea.h>
#include <nmea/nmea_util.h>
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

static unsigned int errors = 0;

static void test_check_fix_zero(void)
{
	int rc;
	int result;
	struct nmea_fix_t t;

	rc = check_fix_zero(NULL);
	result = rc == -1;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.i = 0; t.d = 0;
	rc = check_fix_zero(&t);
	result = rc == 0;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.i = 1; t.d = 0;
	rc = check_fix_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.i = 0; t.d = 1;
	rc = check_fix_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);
}

static void test_check_time_zero(void)
{
	int rc;
	int result;
	struct nmea_time_t t;

	rc = check_time_zero(NULL);
	result = rc == -1;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.h = 0; t.m = 0; t.s = 0; t.ms = 0;
	rc = check_time_zero(&t);
	result = rc == 0;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.h = 1; t.m = 0; t.s = 0; t.ms = 0;
	rc = check_time_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.h = 0; t.m = 1; t.s = 0; t.ms = 0;
	rc = check_time_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.h = 0; t.m = 0; t.s = 1; t.ms = 0;
	rc = check_time_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.h = 0; t.m = 0; t.s = 0; t.ms = 1;
	rc = check_time_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);
}

static void test_check_date_zero(void)
{
	int rc;
	int result;
	struct nmea_date_t t;

	rc = check_date_zero(NULL);
	result = rc == -1;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.y = 0; t.m = 0; t.d = 0;
	rc = check_date_zero(&t);
	result = rc == 0;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.y = 1; t.m = 0; t.d = 0;
	rc = check_date_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.y = 0; t.m = 1; t.d = 0;
	rc = check_date_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);

	t.y = 0; t.m = 0; t.d = 1;
	rc = check_date_zero(&t);
	result = rc == -2;
	if (!result) ++errors;
	printf("%25s : %d : line:%d\n", __FUNCTION__, result, __LINE__);
}

static void test_utils(void)
{
	test_check_fix_zero();
	test_check_time_zero();
	test_check_date_zero();
}

static void test_parse_int(const char * s, int outcome)
{
	uint32_t v;
	int result;
	int len = strlen(s);
	const char * p = parse_int(s, s+len, &v);
	result = outcome == (p == s + len);
	if (!result) ++errors;
	printf("%25s : %d : %d : [%s] : %u\n", __FUNCTION__, result, p == s+len, s, v);
}

static void test_parse_fix(const char * s, int outcome)
{
	struct nmea_fix_t v;
	int result;
	int len = strlen(s);
	const char * p = parse_fix(s, s+len, &v);
	result = outcome == (p == s + len);
	if (!result) ++errors;
	printf("%25s : %d : %d : [%s] : %u %06u\n", __FUNCTION__, result, p == s+len, s, v.i, v.d);
}

static void test_parse_time(const char * s, int outcome)
{
	struct nmea_time_t t;
	int result;
	const char * e = s + strlen(s);
	int r = parse_time(s, e, &t) == e && !check_time(&t);
	result = outcome == r;
	if (!result) ++errors;
	printf("%25s : %d : [%s] : %u %u %u %u\n", __FUNCTION__, result, s, t.h, t.m, t.s, t.ms);
}

static void test_parse_date(const char * s, int outcome)
{
	struct nmea_date_t t;
	int result;
	const char * e = s + strlen(s);
	int r = parse_date(s, e, &t) == e && !check_date(&t);
	result = outcome == r;
	if (!result) ++errors;
	printf("%25s : %d : [%s] : %u %u %u\n", __FUNCTION__, result, s, t.y, t.m, t.d);
}

static void test_parse_lat(const char * s, int outcome)
{
	struct nmea_angle_t t;
	int result;
	const char * e = s + strlen(s);
	int r = parse_angle(s, e, &t) == e && !check_latitude(&t);
	result = outcome == r;
	if (!result) ++errors;
	printf("%25s : %d : [%s] : %u %u %u %u\n", __FUNCTION__, result, s, t.d, t.m, t.s.i, t.s.d);
}

static void test_parse_lon(const char * s, int outcome)
{
	struct nmea_angle_t t;
	int result;
	const char * e = s + strlen(s);
	int r = parse_angle(s, e, &t) == e && !check_longitude(&t);
	result = outcome == r;
	if (!result) ++errors;
	printf("%25s : %d : [%s] : %u %u %u %u\n", __FUNCTION__, result, s, t.d, t.m, t.s.i, t.s.d);
}

static void test_basic_parsing(void)
{
	test_parse_int("0", 1);
	test_parse_int("10", 1);
	test_parse_int("00", 1);
	test_parse_int("0123", 1);
	test_parse_int("9999", 1);
	test_parse_int("10,", 0);
	test_parse_int("", 1);
	test_parse_int(",", 0);
	test_parse_int("0.0", 0);

	test_parse_fix("123456789.1234567", 1);
	test_parse_fix("3.14159265365", 1);
	test_parse_fix("1.2", 1);
	test_parse_fix("3", 1);
	test_parse_fix(".5", 1);
	test_parse_fix("3.", 1);
	test_parse_fix("1.0001", 1);
	test_parse_fix("1.1,", 0);
	test_parse_fix("", 1);
	test_parse_fix(".", 1);

	test_parse_date("010100", 1);
	test_parse_date("999999", 0);
	test_parse_date("320100", 0);
	test_parse_date("000100", 0);
	test_parse_date("011300", 0);
	test_parse_date("311299", 1);
	test_parse_date("999999,", 0);
	test_parse_date("010100,", 0);

	test_parse_time("123456", 1);
	test_parse_time("123456,", 0);
	test_parse_time("123456.345", 1);
	test_parse_time("123456.", 1);
	test_parse_time("0", 1);
	test_parse_time("", 1);

	test_parse_lat("1234.0000", 1);
	test_parse_lat("1234,0000", 0);
	test_parse_lat("1234.0000,", 0);
	test_parse_lat("9000.0000", 1);
	test_parse_lat("0000.0000", 1);
	test_parse_lat("8959.9999", 1);
	test_parse_lat("8960.0000", 0);
	test_parse_lat("9000.1000", 0);

	test_parse_lon("12345.0000", 1);
	test_parse_lon("01234,0000", 0);
	test_parse_lon("01234.0000,", 0);
	test_parse_lon("09000.0000", 1);
	test_parse_lon("00000.0000", 1);
	test_parse_lon("17959.9999", 1);
	test_parse_lon("17960.0000", 0);
	test_parse_lon("18000.0000", 1);
	test_parse_lon("18000.1000", 0);
}

static void test_sentence_parsing()
{
	struct nmea_t info;
	int rc;
	unsigned int i;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(const char *); ++i) {
		memset(&info, 0, sizeof(info));
		rc = nmea_read(&info, SENTENCES[i]);
		if (rc != 0) ++errors;
		printf("%25s : %d : [%s]\n", __FUNCTION__, rc == 0, SENTENCES[i]);
	}
}

static void test_basic_string_writing(void)
{
	enum { SIZE = 128 };

	int rc;
	char buf[SIZE];

	memset(buf, 0, sizeof(buf));
	rc = write_string(buf, SIZE, "");
	if (rc != 0) { ++errors; printf("ERROR: %d\n", __LINE__); }

	memset(buf, 0, sizeof(buf));
	rc = write_string(buf, 0, "hello");
	if (rc != -1) { ++errors; printf("ERROR: %d\n", __LINE__); }

	memset(buf, 0, sizeof(buf));
	rc = write_string(NULL, SIZE, "hello");
	if (rc != -1) { ++errors; printf("ERROR: %d\n", __LINE__); }

	memset(buf, 0, sizeof(buf));
	rc = write_string(buf, SIZE, "hello");
	if (rc != strlen("hello")) { ++errors; printf("ERROR: %d\n", __LINE__); }
}

static void test_write_time(const struct nmea_time_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];
	int result;

	memset(buf, 0, sizeof(buf));
	rc = write_time(buf, SIZE, t);
	result = !strncmp(buf, outcome, SIZE);
	if (!result) ++errors;
	printf("%25s : %d : '%02d %02d %02d %04d' / '%s' ==> '%s'\n", __FUNCTION__,
		result, t->h, t->m, t->s, t->ms, outcome, buf);
}

static void test_write_date(const struct nmea_date_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];
	int result;

	memset(buf, 0, sizeof(buf));
	rc = write_date(buf, SIZE, t);
	result = !strncmp(buf, outcome, SIZE);
	if (!result) ++errors;
	printf("%25s : %d : '%02d %02d %02d' / '%s' ==> '%s'\n", __FUNCTION__,
		result, t->d, t->m, t->d, outcome, buf);
}

static void test_write_lat(const struct nmea_angle_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];
	int result;

	memset(buf, 0, sizeof(buf));
	rc = write_lat(buf, SIZE, t);
	result = !strncmp(buf, outcome, SIZE);
	if (!result) ++errors;
	printf("%25s : %d : '%02d %02d %06d.%06d' / '%s' ==> '%s'\n", __FUNCTION__,
		result, t->d, t->m, t->s.i, t->s.d, outcome, buf);
}

static void test_write_lon(const struct nmea_angle_t * t, const char * outcome)
{
	enum { SIZE = 128 };
	int rc;
	char buf[SIZE];
	int result;

	memset(buf, 0, sizeof(buf));
	rc = write_lon(buf, SIZE, t);
	result = !strncmp(buf, outcome, SIZE);
	if (!result) ++errors;
	printf("%25s : %d : '%03d %02d %06d.%06d' / '%s' ==> '%s'\n", __FUNCTION__,
		result, t->d, t->m, t->s.i, t->s.d, outcome, buf);
}

static void test_basic_time_writing(void)
{
	struct nmea_time_t t;
	t.ms = 0;

	t.h =  0; t.m =  0; t.s =  0; test_write_time(&t, "000000");
	t.h = 10; t.m =  0; t.s =  0; test_write_time(&t, "100000");
	t.h = 23; t.m =  0; t.s =  0; test_write_time(&t, "230000");
	t.h = 24; t.m =  0; t.s =  0; test_write_time(&t, "");
	t.h =  0; t.m =  0; t.s =  0; test_write_time(&t, "000000");
	t.h = 10; t.m = 10; t.s =  0; test_write_time(&t, "101000");
	t.h = 23; t.m = 59; t.s =  0; test_write_time(&t, "235900");
	t.h = 24; t.m = 60; t.s =  0; test_write_time(&t, "");
	t.h =  0; t.m =  0; t.s =  0; test_write_time(&t, "000000");
	t.h = 10; t.m = 10; t.s = 10; test_write_time(&t, "101010");
	t.h = 23; t.m = 59; t.s = 59; test_write_time(&t, "235959");
	t.h = 24; t.m = 60; t.s = 60; test_write_time(&t, "");
}

static void test_basic_date_writing(void)
{
	struct nmea_date_t t;

	t.y =   0; t.m =  0; t.d =  0; test_write_date(&t, "");
	t.y =   0; t.m =  1; t.d =  1; test_write_date(&t, "010100");
	t.y =  99; t.m =  1; t.d =  1; test_write_date(&t, "010199");
	t.y = 100; t.m =  1; t.d =  1; test_write_date(&t, "010100");
	t.y =   0; t.m =  0; t.d =  1; test_write_date(&t, "");
	t.y =  99; t.m =  1; t.d =  1; test_write_date(&t, "010199");
	t.y = 100; t.m = 12; t.d =  1; test_write_date(&t, "011200");
	t.y =   0; t.m = 13; t.d =  1; test_write_date(&t, "");
	t.y =   0; t.m =  1; t.d =  1; test_write_date(&t, "010100");
	t.y =   0; t.m =  1; t.d = 31; test_write_date(&t, "310100");
	t.y =   0; t.m =  1; t.d = 32; test_write_date(&t, "");
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
	t.d =  1; t.m =  0; t.s.i =  1; test_write_lat(&t, "0100.0000");
	t.d = 89; t.m =  0; t.s.i = 59; test_write_lat(&t, "8900.0000");
	t.d = 90; t.m =  0; t.s.i = 60; test_write_lat(&t, "");
	t.d = 89; t.m =  0; t.s.i = 61; test_write_lat(&t, "");
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
	t.d =   1; t.m =  0; t.s.i =  1; test_write_lon(&t, "00100.0000");
	t.d = 179; t.m =  0; t.s.i = 59; test_write_lon(&t, "17900.0000");
	t.d =   0; t.m =  0; t.s.i = 60; test_write_lon(&t, "");
	t.d = 181; t.m =  0; t.s.i = 61; test_write_lon(&t, "");
}

static void test_basic_writing(void)
{
	test_basic_string_writing();
	test_basic_time_writing();
	test_basic_date_writing();
	test_basic_latitude_writing();
	test_basic_longitude_writing();
}

static void test_writing_sentence(const char * s)
{
	int rc;
	char buf[128];
	struct nmea_t nmea;
	int result;

	memset(buf, 0, sizeof(buf));
	memset(&nmea, 0, sizeof(nmea));
	rc = nmea_read(&nmea, s);
	if (rc != 0) {
		++errors;
		printf("%25s : %d : [%s] : unable to parse\n", __FUNCTION__, 0, s);
	}
	rc = nmea_write(buf, sizeof(buf), &nmea);
	result = (rc >= 0 && !strcmp(s, buf));
	if (!result) ++errors;
	printf("%25s : %d : [%s]\n%25s       [%s]\n%25s       rc=%d\n", __FUNCTION__, result, s, "", buf, "", rc);
}

static void test_sentence_writing(void)
{
	unsigned int i;

	for (i = 0; i < sizeof(SENTENCES)/sizeof(const char *); ++i) {
		test_writing_sentence(SENTENCES[i]);
	}
}

int main(int argc, char ** argv)
{
	int test = 0;
	if (argc > 1) test = atoi(argv[1]);

	switch (test) {
		case 0:
			test_utils();
			test_basic_parsing();
			test_basic_writing();
			test_sentence_parsing();
			test_sentence_writing();
			break;
		case 1:
			test_utils();
			break;
		case 2:
			test_basic_parsing();
			break;
		case 3:
			test_basic_writing();
			break;
		case 4:
			test_sentence_parsing();
			break;
		case 5:
			test_sentence_writing();
			break;
	}

	if (errors > 0) {
		printf("\nERRORS: %u\n\n", errors);
	}

	return 0;
}

