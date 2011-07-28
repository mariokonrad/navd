#ifndef __NMEA_UTIL__H__
#define __NMEA_UTIL__H__

#include <nmea0183/nmea.h>

uint8_t checksum(const char * s, const char * e);
int check_checksum(const char * s, char start_token);
int check_fix_zero(const struct nmea_fix_t * v);
int check_time_zero(const struct nmea_time_t * v);
int check_time(const struct nmea_time_t * v);
int check_date_zero(const struct nmea_date_t * v);
int check_date(const struct nmea_date_t * v);
int check_angle_zero(const struct nmea_angle_t * v);
int check_latitude(const struct nmea_angle_t * v);
int check_longitude(const struct nmea_angle_t * v);
const char * parse_str(const char * s, const char * e, char * v);
const char * parse_int(const char * s, const char * e, uint32_t * v);
const char * parse_fix(const char * s, const char * e, struct nmea_fix_t * v);
const char * parse_time(const char * s, const char * e, struct nmea_time_t * v);
const char * parse_date(const char * s, const char * e, struct nmea_date_t * v);
const char * parse_angle(const char * s, const char * e, struct nmea_angle_t * v);
int write_string(char * buf, uint32_t size, const char * s);
int write_char(char * buf, uint32_t size, const char c);
int write_fix(char * buf, uint32_t size, const struct nmea_fix_t * v, uint32_t ni, uint32_t nd);
int write_time(char * buf, uint32_t size, const struct nmea_time_t * v);
int write_date(char * buf, uint32_t size, const struct nmea_date_t * v);
int write_lat(char * buf, uint32_t size, const struct nmea_angle_t * v);
int write_lon(char * buf, uint32_t size, const struct nmea_angle_t * v);
int write_checksum(char * buf, uint32_t size, const char * s, const char * e);

int nmea_fix_to_float(float *, const struct nmea_fix_t *);
int nmea_fix_to_double(double *, const struct nmea_fix_t *);

const char * find_token_end(const char * s);
const char * find_sentence_end(const char * s);
int token_valid(const char * s, const char * p);

#endif
