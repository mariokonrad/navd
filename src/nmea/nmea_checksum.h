#ifndef __NMEA_CHECKSUM__H__
#define __NMEA_CHECKSUM__H__

#include <stdint.h>

uint8_t checksum(const char * s, const char * e);
int check_checksum(const char * s, char start_token);
int write_checksum(char * buf, uint32_t size, const char * s, const char * e);

#endif
