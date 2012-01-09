#ifndef __NMEA__H__
#define __NMEA__H__

#include <nmea/nmea_base.h>

int nmea_read(struct nmea_t *, const char *);
int nmea_write(char *, uint32_t, const struct nmea_t *);
int nmea_hton(struct nmea_t *);
int nmea_ntoh(struct nmea_t *);

const struct nmea_sentence_t * nmea_sentence(uint32_t);

#endif
