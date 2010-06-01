#include "nmea.h"
#include <stdio.h>
#include <string.h>

static uint8_t hex2i(char c)
{
	if ((c >= '0') && (c <= '9')) return c - '0';
	if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
	if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
	return 0xff;
}

static int parse_fix(const char * s, int len, uint32_t * val, uint8_t * decimal)
{
	unsigned int i;
	int dec_inc = 0;

	*val = 0;
	*decimal = 0;
	for (i = 0; s && *s && i < len; ++i, ++s) {
		if (*s == '.') {
			dec_inc = 1;
		} else {
			*val *= 10;
			*val += *s - '0';
			*decimal += dec_inc;
		}
	}
	return i;
}

/*
 * @retval  0 success
 * @retval -1 failure
 */
static int checksum(const char * s)
{
	uint8_t chk = 0;
	if (!s || !(*s) || *s != '$') return -1;
	++s; /* skip '$' */
	for (; *s && *s != '*'; ++s) chk ^= *s;
	++s; /* skip '*' */
	return chk == hex2i(s[0]) * 16 + hex2i(s[1]) ? 0 : -1;
}

static const char * find_comma(const char * s)
{
	while (s && *s && *s != ',') ++s;
	return s;
}

/*
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_time(const char * s, struct nmea_time_t * time)
{
	if (s) {
		time->h = (s[0]-'0') * 10 + (s[1]-'0');
		time->m = (s[2]-'0') * 10 + (s[3]-'0');
		time->s = (s[4]-'0') * 10 + (s[5]-'0');
		return (time->h > 23 || time->m > 59 || time->s > 59) ? -1 : 0;
	} else {
		time->h = 0;
		time->m = 0;
		time->s = 0;
		return 0;
	}
}

/*
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_date(const char * s, struct nmea_date_t * date)
{
	if (s) {
		date->d = (s[0]-'0') * 10 + (s[1]-'0');
		date->m = (s[2]-'0') * 10 + (s[3]-'0');
		date->y = (s[4]-'0') * 10 + (s[5]-'0');
		return (date->m > 12 || date->m == 0 || date->d > 31 || date->d == 0) ? -1 : 0;
	} else {
		date->y = 0;
		date->m = 0;
		date->d = 0;
		return 0;
	}
}

static int parse_latitude(const char * s, struct nmea_angle_t * angle)
{
	if (s) {
		/* TODO: parse_fix */
		angle->d = (s[0]-'0') * 10 + (s[1]-'0');
		angle->m = (s[2]-'0') * 10 + (s[3]-'0');
		angle->s = 60 * ((s[5]-'0') * 1000 + (s[6]-'0') * 100 + (s[7]-'0') * 10 + (s[8]-'0'));
		return (angle->d > 90 || angle->m > 59 || angle->s > 599999) ? -1 : 0;
	} else {
		angle->d = 0;
		angle->m = 0;
		angle->s = 0;
		return 0;
	}
}

static int parse_longitude(const char * s, struct nmea_angle_t * angle)
{
	if (s) {
		/* TODO: parse_fix */
		angle->d = (s[0]-'0') * 100 + (s[1]-'0') * 10 + (s[2]-'0');
		angle->m = (s[3]-'0') * 10 + (s[4]-'0');
		angle->s = 60 * ((s[6]-'0') * 1000 + (s[7]-'0') * 100 + (s[8]-'0') * 10 + (s[9]-'0'));
		return (angle->d > 180 || angle->m > 59 || angle->s > 599999) ? -1 : 0;
	} else {
		angle->d = 0;
		angle->m = 0;
		angle->s = 0;
		return 0;
	}
}

/*
 * @retval  0 success
 * @retval -1 failure
 */
static int parse_gprmc(const char * s, struct nmea_t * nmea) /* {{{ */
{
	int state = 0;
	const char * p = s;
	struct nmea_rmc_t * rmc = &nmea->sentence.rmc;
	nmea->type = NMEA_RMC;
	while (*s && *p && *s != '*' && state >= 0) {
/*
		printf("%s:%d: s:[%s]\n", __PRETTY_FUNCTION__, __LINE__, s);
*/
		p = find_comma(s);
		switch (state) {
			case 0: /* time */
				if (parse_time((s == p) ? NULL : s, &rmc->time)) return -1;
				break;
			case 1: /* state */
				rmc->status = (s == p) ? NMEA_STATUS_WARNING : *s;
				break;
			case 2: /* latitude */
				if (parse_latitude((s == p) ? NULL : s, &rmc->lat)) return -1;
				break;
			case 3: /* latitude direction */
				rmc->lat_dir = (s == p) ? NMEA_NORTH : *s;
				break;
			case 4: /* longitude */
				if (parse_longitude((s == p) ? NULL : s, &rmc->lon)) return -1;
				break;
			case 5: /* longitude direction */
				rmc->lon_dir = (s == p) ? NMEA_EAST : *s;
				break;
			case 6: /* velocity in knots */
		/* TODO: parse_fix */
				rmc->sog = (s == p) ? 0 : (s[0]-'0') * 100 + (s[1]-'0') * 10 + (s[3]-'0');
				printf("dbg: sog:%03d\n", rmc->sog);
				break;
			case 7: /* heading over ground regarding geographic nord */
		/* TODO: parse_fix */
				rmc->head = (s == p) ? 0 : (s[0]-'0') * 1000 + (s[1]-'0') * 100 + (s[2]-'0') * 10 + (s[4]-'0');
				printf("dbg: head:%04d\n", rmc->head);
				break;
			case 8: /* date */
				if (parse_date((s == p) ? NULL : s, &rmc->date)) return -1;
				break;
			case 9: /* magnetic deviation */
		/* TODO: parse_fix */
				if (s == p) {
					rmc->m = 0;
				} else {
					/* TODO: set magnetic deviation */
/*
					printf("%s:%d:\n", __PRETTY_FUNCTION__, __LINE__);
*/
				}
				break;
			case 10: /* magnetic deviation direction */
				rmc->m_dir = (s == p) ? NMEA_EAST : *s;
				break;
			case 11:
				rmc->sig_integrity = (s == p) ? NMEA_SIG_INT_DATANOTVALID : *s;
				state = -2;
				break;
		}
		s = p+1;
		++state;
	}
	return 0;
} /* }}} */

/*
 * @param[in]  s read sentence
 * @param[out] nmea data of the parsed structure
 * @retval  0 success
 * @retval  1 unknown sentence
 * @retval -1 parameter error
 * @retval -2 checksum error
 */
int nmea_read(const char * s, struct nmea_t * nmea)
{
	struct entry_t {
		const char * tag;
		int (*parser)(const char *, struct nmea_t *);
	};

	static const struct entry_t TAB[] = {
		{ "GPRMC", parse_gprmc },
		{ NULL,    NULL        }
	};

	const char * p = s;
	const struct entry_t * entry = NULL;

	if (!s || !nmea) return -1;
	if (checksum(s)) return -2;
	if (*s != '$') return -1;
	++s;
	p = find_comma(s);
	for (entry = TAB; entry && entry->tag; ++entry) {
		if (!strncmp(s, entry->tag, p-s)) {
			return entry->parser(p+1, nmea) ? -1 : 0;
		}
	}
	return 1;
}

static void test(const char * s)
{
	uint32_t val;
	uint8_t dec;
	int rc;
	rc = parse_fix(s, strlen(s), &val, &dec);
	printf("test: [%s]  len=%d  rc=%d  val=%08d, dec=%d\n", s, strlen(s), rc, val, dec);
}

int main() /* TEMP {{{ */
{
	test("0.0");
	test("1.000");
	test("100.000");
	test("328.4");

	return -1;

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
/* {{{
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
}}} */
	};
	struct nmea_t info;
	int rc;
	unsigned int i;

	for (i = 0; i < sizeof(S)/sizeof(const char *); ++i) {
		rc = nmea_read(S[i], &info);
		printf("rc=%2d  [%s]\n", rc, S[i]);
	}

	return 0;
} /* }}} */

