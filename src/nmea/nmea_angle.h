#ifndef __NMEA_ANGLE__H__
#define __NMEA_ANGLE__H__

#include <nmea/nmea_fix.h>

#define NMEA_ANGLE_DECIMALS 10000
#define NMEA_ANGLE_DECIMAL_DIGITS 4

/**
 * Represents an angle used by the NMEA functions. This structure
 * supports degrees, minutes and seconds (plus fractions of them).
 */
struct nmea_angle_t {
	uint32_t d; /* degrees */
	uint32_t m; /* minutes */
	struct nmea_fix_t s; /* seconds */
};

int nmea_angle_check_zero(const struct nmea_angle_t * v);
int nmea_check_latitude(const struct nmea_angle_t * v);
int nmea_check_longitude(const struct nmea_angle_t * v);
const char * nmea_angle_parse(const char * s, const char * e, struct nmea_angle_t * v);
int nmea_write_latitude(char * buf, uint32_t size, const struct nmea_angle_t * v);
int nmea_write_lonitude(char * buf, uint32_t size, const struct nmea_angle_t * v);
void nmea_angle_hton(struct nmea_angle_t * v);
void nmea_angle_ntoh(struct nmea_angle_t * v);

int nmea_angle_to_double(double *, const struct nmea_angle_t *);

#endif
