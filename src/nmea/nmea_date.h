#ifndef __NMEA_DATE__H__
#define __NMEA_DATE__H__

#include <stdint.h>

struct nmea_date_t {
	uint32_t y; /* year */
	uint32_t m; /* month: 1..12 */
	uint32_t d; /* day: 1..31 */
};

int check_date_zero(const struct nmea_date_t * v);
int check_date(const struct nmea_date_t * v);
const char * parse_date(const char * s, const char * e, struct nmea_date_t * v);
int write_date(char * buf, uint32_t size, const struct nmea_date_t * v);
void nmea_date_hton(struct nmea_date_t * v);
void nmea_date_ntoh(struct nmea_date_t * v);

#endif
