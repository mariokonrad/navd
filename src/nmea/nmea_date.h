#ifndef __NMEA_DATE__H__
#define __NMEA_DATE__H__

#include <stdint.h>

/**
 * @todo Documenation
 */
struct nmea_date_t {
	uint32_t y; /* year */
	uint32_t m; /* month: 1..12 */
	uint32_t d; /* day: 1..31 */
};

int nmea_date_check_zero(const struct nmea_date_t * v);
int nmea_date_check(const struct nmea_date_t * v);
const char * nmea_date_parse(const char * s, const char * e, struct nmea_date_t * v);
int nmea_date_write(char * buf, uint32_t size, const struct nmea_date_t * v);
void nmea_date_hton(struct nmea_date_t * v);
void nmea_date_ntoh(struct nmea_date_t * v);

#endif
