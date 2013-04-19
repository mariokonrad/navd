#ifndef __NMEA_TIME__H__
#define __NMEA_TIME__H__

#include <stdint.h>

/**
 * Date information for NMEA data handling.
 */
struct nmea_time_t {
	uint32_t h;  /* hour: 0..23 */
	uint32_t m;  /* minute: 0..59 */
	uint32_t s;  /* second: 0..59 */
	uint32_t ms; /* millisecond: 0..999 */
};

int nmea_time_check_zero(const struct nmea_time_t * v);
int nmea_time_check(const struct nmea_time_t * v);
const char * nmea_time_parse(const char * s, const char * e, struct nmea_time_t * v);
int nmea_time_write(char * buf, uint32_t size, const struct nmea_time_t * v);
void nmea_time_hton(struct nmea_time_t *);
void nmea_time_ntoh(struct nmea_time_t *);

#endif
