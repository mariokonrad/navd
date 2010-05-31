#ifndef __NMEA__H__
#define __NMEA__H__

#include <stdint.h>

#define NMEA_RMA 0x0001
#define NMEA_RMB 0x0002
#define NMEA_RMC 0x0003
#define NMEA_GGA 0x0004
#define NMEA_GSA 0x0005
#define NMEA_GSV 0x0006

#define NMEA_EAST  'E'
#define NMEA_WEST  'W'
#define NMEA_NORTH 'N'
#define NEMA_SOUGH 'S'

#define NMEA_STATUS_OK      'A'
#define NMEA_STATUS_WARNING 'V'

#define NMEA_SIG_INT_AUTONOMOUS   'A'
#define NMEA_SIG_INT_DIFFERENTIAL 'D'
#define NMEA_SIG_INT_ESTIMATED    'E'
#define NMEA_SIG_INT_MANUALINPUT  'M'
#define NMEA_SIG_INT_SIMULATED    'S'
#define NMEA_SIG_INT_DATANOTVALID 'N'

struct nmea_time_t {
	uint8_t hh;
	uint8_t mm;
	uint8_t ss;
};

struct nmea_date_t {
	uint16_t yy;
	uint8_t  mm;
	uint8_t  dd;
};

struct nmea_rmc_t {
	struct nmea_time_t time; /* HHMMSS */
	char status; /* A:OK, V:WARNING */
	/* BBBB.BBBB (LATITUDE) */ /* TODO */
	char lat_dir; /* N:NORTH, S:SOUTH */
	/* LLLLL.LLLL (LONGITUDE) */ /* TODO */
	char lon_dir; /* E:EAST, W:WEST */
	uint16_t sog; /* fixpoint, GG.G (SPEED OVER GROUND IN KNOTS) */
	uint16_t heading; /* fixpoint, RR.R (HEADING OVER GROUND IN DEGREES REGARDING GEOGRAPIC NORTH) */
	struct nmea_date_t date; /* DDMMYY */
	uint8_t m; /* fixpoint, M.M (MAGNETIC DEVIATION) */
	char m_dir; /* MAGNETIC DEVIATION E:EAST, W:WEST */
	char sig_integrity; /* SIGNAL INTEGRITY MODE, A:AUTONOMOUS, D:DIFFERENTIAL, E:ESTIMATED, M:MANUAL INPUT, S:SIMULATED, N:DATA NOT VALID */
};

struct nmea_t {
	uint16_t type;
	union {
		struct nmea_rmc_t rmc;
	} sentence;
};

int nmea_read(const char *, struct nmea_t *);

#endif
