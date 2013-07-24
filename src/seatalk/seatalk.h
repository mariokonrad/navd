#ifndef __SEATALK__H__
#define __SEATALK__H__

#include <seatalk/seatalk_base.h>

int seatalk_read(struct seatalk_t *, const char *, uint32_t);
int seatalk_write(char *, uint32_t, const struct seatalk_t *);
int seatalk_hton(struct seatalk_t *);
int seatalk_ntoh(struct seatalk_t *);

const struct seatalk_sentence_t * seatalk_sentence(uint8_t);

#endif
