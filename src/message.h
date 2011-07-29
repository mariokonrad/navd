#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#include <nmea/nmea.h>

struct message_t
{
	uint32_t type;
	union {
		struct nmea_t nmea;
		int8_t buf[sizeof(struct nmea_t)]; /* TODO: max size of all members */
	} data;
};

#endif
