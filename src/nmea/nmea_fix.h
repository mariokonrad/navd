#ifndef __NMEA_FIX__H__
#define __NMEA_FIX__H__

#include <stdint.h>

#define NMEA_FIX_DECIMALS 1000000
#define NMEA_FIX_DECIMAL_DIGITS 6

struct nmea_fix_t { /* x.xxxxxx */
	uint32_t i; /* integer part, max. 6 digits, see NMEA_FIX_DECIMALS */
	uint32_t d; /* decimal part, 6 digits, see NMEA_FIX_DECIMALS */
};

int nmea_fix_check_zero(const struct nmea_fix_t * v);
const char * nmea_fix_parse(const char * s, const char * e, struct nmea_fix_t * v);
int nmea_fix_write(char * buf, uint32_t size, const struct nmea_fix_t * v, uint32_t ni, uint32_t nd);
void nmea_fix_hton(struct nmea_fix_t *);
void nmea_fix_ntoh(struct nmea_fix_t *);

int nmea_fix_to_float(float *, const struct nmea_fix_t *);
int nmea_fix_to_double(double *, const struct nmea_fix_t *);

#endif
