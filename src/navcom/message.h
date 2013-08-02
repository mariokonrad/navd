#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#include <stdint.h>
#include <global_config.h>

#if defined(NEEDS_NMEA)
	#include <nmea/nmea.h>
#endif

#if defined(NEEDS_SEATALK)
	#include <seatalk/seatalk.h>
#endif

/**
 * Message types, defines what type of message it is.
 *
 * @note Values within 0x00ffffff are reserved for system messages,
 *   while values within 0xyyffffff (with y=0x01..0xff) are for application purposes.
 */
enum MessageType {
	/** Invalid message */
	 MSG_INVALID = 0x00000000

	/** System message */
	,MSG_SYSTEM  = 0x00000001

	/** Messages from timers */
	,MSG_TIMER   = 0x01000000

	/** Navigational messages */
	,MSG_NMEA    = 0x01000001

	/** Navigational messages */
	,MSG_SEATALK = 0x01000002
};

/**
 * Enumeration of system messages.
 */
enum System {
	/** Graceful termination of the system */
	SYSTEM_TERMINATE = 0x00000001
};

/**
 * Structure which contains all possible data for a message to
 * be transmitted.
 */
struct message_data_t
{
	uint32_t system; /* see enum System */
	uint32_t timer_id;

#if defined(NEEDS_NMEA)
	struct nmea_t nmea;
#endif

#if defined(NEEDS_SEATALK)
	struct seatalk_t seatalk;
#endif
} __attribute__((packed));

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
		struct message_data_t attr;
		int8_t buf[sizeof(struct message_data_t)];
	} data;
} __attribute__((packed));

#endif
