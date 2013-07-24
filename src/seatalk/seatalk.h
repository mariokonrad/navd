#ifndef __SEATALK__H__
#define __SEATALK__H__

#include <seatalk/seatalk_base.h>

int seatalk_read(struct seatalk_t *, const char *, unsigned int);
int seatalk_hton(struct seatalk_t *);
int seatalk_ntoh(struct seatalk_t *);

#endif
