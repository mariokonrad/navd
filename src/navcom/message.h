#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#include <nmea/nmea.h>

/**
 * @todo Documentation
 */
enum MessageType {
	 MSG_SYSTEM = 0x00000000
	,MSG_NMEA   = 0x00010000
};

/**
 * @todo Documentation
 */
enum System {
	SYSTEM_TERMINATE = 0x00000000
};

/**
 * @todo Documentation
 */
struct message_t
{
	uint32_t type;
	union {
		uint32_t system;
		struct nmea_t nmea;
		int8_t buf[sizeof(struct nmea_t)]; /* TODO: max size of all members */
	} data;
};

#endif
