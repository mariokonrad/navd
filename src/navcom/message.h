#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#include <nmea/nmea.h>

/**
 * Message types, defines what type of message it is.
 *
 * @note Values within 0x00ffffff are reserved for system messages,
 *   while values within 0xyyffffff (with y=0x01..0xff) are for application purposes.
 */
enum MessageType {
	 MSG_SYSTEM = 0x00000000
	,MSG_TIMER  = 0x01000000
	,MSG_NMEA   = 0x01000001
};

/**
 * @todo Documentation
 */
enum System {
	SYSTEM_TERMINATE = 0x00000000
};

/**
 * Structure to represent the date to be sent as message.
 * This structure can hold any possible data to be sent as message,
 * therefore every message in this system has the same size. This
 * is a tradeoff between resource management and easy to maintain.
 */
struct message_t
{
	uint32_t type; /* see enum MessageType */
	union {
		uint32_t system; /* see enum System */
		uint32_t timer_id;
		struct nmea_t nmea;
		int8_t buf[sizeof(struct nmea_t)]; /* max size of all members */
	} data;
} __attribute__((packed));

#endif
