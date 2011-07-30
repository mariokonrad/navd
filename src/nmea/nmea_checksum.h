#ifndef __NMEA_CHECKSUM__H__
#define __NMEA_CHECKSUM__H__

#include <stdint.h>

uint8_t nmea_checksum(const char * s, const char * e);
int nmea_checksum_check(const char * s, char start_token);
int nmea_checksum_write(char * buf, uint32_t size, const char * s, const char * e);

#endif
