#ifndef __NMEA_ANGLE__H__
#define __NMEA_ANGLE__H__

#include <nmea/nmea_fix.h>

#define NMEA_ANGLE_DECIMALS 10000
#define NMEA_ANGLE_DECIMAL_DIGITS 4

struct nmea_angle_t {
	uint32_t d; /* degrees */
	uint32_t m; /* minutes */
	struct nmea_fix_t s; /* seconds */
};

int check_angle_zero(const struct nmea_angle_t * v);
int check_latitude(const struct nmea_angle_t * v);
int check_longitude(const struct nmea_angle_t * v);
const char * parse_angle(const char * s, const char * e, struct nmea_angle_t * v);
int write_lat(char * buf, uint32_t size, const struct nmea_angle_t * v);
int write_lon(char * buf, uint32_t size, const struct nmea_angle_t * v);
void nmea_angle_hton(struct nmea_angle_t * v);
void nmea_angle_ntoh(struct nmea_angle_t * v);

#endif
