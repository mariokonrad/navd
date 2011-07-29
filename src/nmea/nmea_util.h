#ifndef __NMEA_UTIL__H__
#define __NMEA_UTIL__H__

#include <stdint.h>

const char * parse_str(const char * s, const char * e, char * v);
int write_string(char * buf, uint32_t size, const char * s);
int write_char(char * buf, uint32_t size, const char c);

const char * find_token_end(const char * s);
const char * find_sentence_end(const char * s);
int token_valid(const char * s, const char * p);

#endif
