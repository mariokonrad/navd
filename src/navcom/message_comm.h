#ifndef __MESSAGE_COMM__H__
#define __MESSAGE_COMM__H__

#include <navcom/message.h>

int message_read(int, struct message_t *);
int message_write(int, const struct message_t *);

#endif
