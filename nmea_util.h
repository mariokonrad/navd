#ifndef __NMEA_UTIL__H__
#define __NMEA_UTIL__H__

#include "nmea.h"

int nmea_fix_to_float(const struct nmea_fix_t *, float *);
int nmea_fix_to_double(const struct nmea_fix_t *, double *);

#endif
