#include "nmea.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void test_parse_int(const char * s, int outcome)
{
	uint32_t v;
	int len = strlen(s);
	const char * p = parse_int(s, s+len, &v);
	printf("%12s : %2d %2d => %d : [%s] : %u\n", __FUNCTION__, outcome, p == s+len, outcome == (p==s+len), s, v);
}

static void test_parse_fix(const char * s, int outcome)
{
	struct nmea_fix_t v;
	int len = strlen(s);
	const char * p = parse_fix(s, s+len, &v);
	printf("%12s : %2d %2d => %d : [%s] : %u %06u\n", __FUNCTION__, outcome, p == s+len, outcome == (p==s+len), s, v.i, v.d);
}

static void test_parse_time(const char * s, int outcome)
{
	struct nmea_time_t t;
	const char * e = s + strlen(s);
	int r = parse_time(s, e, &t) == e && !check_time(&t);
	printf("%12s : %2d %2d => %d : [%s] : %02u : %02u : %02u : %03u\n", __FUNCTION__, outcome, r, outcome == r, s, t.h, t.m, t.s, t.ms);
}

static void test_parse_date(const char * s, int outcome)
{
	struct nmea_date_t t;
	const char * e = s + strlen(s);
	int r = parse_date(s, e, &t) == e && !check_date(&t);
	printf("%12s : %2d %2d => %d : [%s] : %02u : %02u : %02u\n", __FUNCTION__, outcome, r, outcome == r, s, t.y, t.m, t.d);
}

static void test_parse_lat(const char * s, int outcome)
{
	struct nmea_angle_t t;
	const char * e = s + strlen(s);
	int r = parse_angle(s, e, &t) == e && !check_latitude(&t);
	printf("%12s : %2d %2d => %d : [%s] : %02u : %02u : %u : %u\n", __FUNCTION__, outcome, r, outcome == r, s, t.d, t.m, t.s.i, t.s.d);
}

static void test_parse_lon(const char * s, int outcome)
{
	struct nmea_angle_t t;
	const char * e = s + strlen(s);
	int r = parse_angle(s, e, &t) == e && !check_longitude(&t);
	printf("%12s : %2d %2d => %d : [%s] : %02u : %02u : %u : %u\n", __FUNCTION__, outcome, r, outcome == r, s, t.d, t.m, t.s.i, t.s.d);
}

static void test_basic_parsing(void) /* {{{ */
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
	printf("\n");

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
	printf("\n");

	test_parse_date("010100", 1);
	test_parse_date("999999", 0);
	test_parse_date("320100", 0);
	test_parse_date("000100", 0);
	test_parse_date("011300", 0);
	test_parse_date("311299", 1);
	test_parse_date("999999,", 0);
	test_parse_date("010100,", 0);
	printf("\n");

	test_parse_time("123456", 1);
	test_parse_time("123456,", 0);
	test_parse_time("123456.345", 1);
	test_parse_time("123456.", 1);
	test_parse_time("0", 1);
	test_parse_time("", 1);
	printf("\n");

	test_parse_lat("1234.0000", 1);
	test_parse_lat("1234,0000", 0);
	test_parse_lat("1234.0000,", 0);
	test_parse_lat("9000.0000", 0);
	test_parse_lat("0000.0000", 1);
	test_parse_lat("8959.9999", 1);
	test_parse_lat("8960.0000", 0);
	printf("\n");

	test_parse_lon("12345.0000", 1);
	test_parse_lon("01234,0000", 0);
	test_parse_lon("01234.0000,", 0);
	test_parse_lon("09000.0000", 1);
	test_parse_lon("00000.0000", 1);
	test_parse_lon("17959.9999", 1);
	test_parse_lon("17960.0000", 0);
	printf("\n");
} /* }}} */

static void test_sentence_parsing() /* {{{ */
{
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
	static const char * S[] = {
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

	struct nmea_t info;
	int rc;
	unsigned int i;

	for (i = 0; i < sizeof(S)/sizeof(const char *); ++i) {
		rc = nmea_read(&info, S[i]);
		printf("rc=%2d  [%s]\n", rc, S[i]);
	}
} /* }}} */

static void test_basic_writing(void)
{
	/* TODO */
}

int main(int argc, char ** argv)
{
	int test = 0;
	if (argc > 1) test = atoi(argv[1]);

	switch (test) {
		case 0: test_basic_parsing(); break;
		case 1: test_sentence_parsing(); break;
		case 2: test_basic_writing(); break;
	}

	return 0;
}

