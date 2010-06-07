#ifndef __NMEA__H__
#define __NMEA__H__

#include <stdint.h>

#define NMEA_RMA 0x0001
#define NMEA_RMB 0x0002
#define NMEA_RMC 0x0003
#define NMEA_GGA 0x0004
#define NMEA_GSA 0x0005
#define NMEA_GSV 0x0006
#define NMEA_GLL 0x0007

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

#define NMEA_QUALITY_INVALID  0
#define NMEA_QUALITY_GPS_FIX  1
#define NMEA_QUALITY_DPGS_FIX 2
#define NMEA_QUALITY_GUESS    6

#define NMEA_UNIT_METER 'M'

#define NMEA_LEFT  'L'
#define NMEA_RIGHT 'R'

struct nmea_fix_t { /* x.xxxxxx */
	uint32_t i;
	uint32_t d;
};

struct nmea_time_t {
	uint32_t h;
	uint32_t m;
	uint32_t s;
	uint32_t ms;
};

struct nmea_date_t {
	uint32_t y;
	uint32_t m;
	uint32_t d;
};

struct nmea_angle_t {
	uint32_t d;
	uint32_t m;
	float s;
};

struct nmea_satelite_t {
	uint32_t id;
	uint32_t elevation;
	uint32_t azimuth; /* to true */
	uint32_t snr;
};

struct nmea_rmb_t {
	char status; /* V:warning */
	float cross_track_error; /* cross track error in nautical miles */
	char steer_dir; /* direction to steer, left or right */
	uint32_t waypoint_to; /* TO waypoint ID */
	uint32_t waypoint_from; /* FROM waypoint ID */
	struct nmea_angle_t lat; /* destination waypoint latitude */
	char lat_dir; /* destination waypoint latitude dir, N:north, S:south */
	struct nmea_angle_t lon; /* destination waypoint longitude */
	char lon_dir; /* destination waypoint longitude dir, E:east, W:west */
	float range; /* range to destination in nautical miles */
	float bearing; /* bearing to destination in degrees to true */
	float dst_velocity; /* destination closing velocity in knots */
	char arrival_status; /* arrival status, A:arrival circle entered */
};

struct nmea_rmc_t {
	struct nmea_time_t time;
	char status; /* A:ok, V:warning */
	struct nmea_angle_t lat; /* BBBB.BBBB */
	char lat_dir; /* N:north, S:south */
	struct nmea_angle_t lon; /* LLLLL.LLLL */
	char lon_dir; /* E:east, W:west */
	float sog; /* speed over ground in knots */
	float head; /* heading over ground in degrees regarding geograpic north */
	struct nmea_date_t date; /* DDMMYY */
	float m; /* magnetic deviation */
	char m_dir; /* magnetic deviation E:east, W:west */
	char sig_integrity; /* signal integrity mode, A:autonomous, D:differential, E:estimated, M:manual input, S:simulated, N:data not valid */
};

struct nmea_gga_t {
	struct nmea_time_t time;
	struct nmea_angle_t lat; /* BBBB.BBBB */
	char lat_dir; /* N:north, S:south */
	struct nmea_angle_t lon; /* LLLLL.LLLL */
	char lon_dir; /* E:east, W:west */
	uint32_t quality;
	uint32_t n_satelites;
	float hor_dilution; /* horizontal dilution of precision */
	float height_antenna; /* height of antenna over geoid */
	char unit_antenna; /* M:meter */
	float geodial_separation; /* geodial separation, sea level below the ellipsoid */
	char unit_geodial_separation; /* M:meter */
	float dgps_age; /* age of dgps data */
	uint32_t dgps_ref; /* dgps reference station 0000..1023 */
};

struct nmea_gsa_t {
	char selection_mode;
	uint32_t mode;
	uint32_t id[12];
	float pdop;
	float hdop;
	float vdop;
};

struct nmea_gsv_t {
	uint32_t n_messages;
	uint32_t message_number;
	struct nmea_satelite_t sat[4]; /* max satelites per message */
};

struct nmea_gll_t {
	struct nmea_angle_t lat;
	char lat_dir;
	struct nmea_angle_t lon;
	char lon_dir;
	struct nmea_time_t time; /* utc */
	char status;
};

struct nmea_t {
	uint32_t type;
	union {
		struct nmea_rmb_t rmb;
		struct nmea_rmc_t rmc;
		struct nmea_gga_t gga;
		struct nmea_gsv_t gsv;
		struct nmea_gsa_t gsa;
		struct nmea_gll_t gll;
	} sentence;
};

int nmea_read(const char *, struct nmea_t *);

/* TEMP */
int check_time(const struct nmea_time_t * v);
int check_date(const struct nmea_date_t * v);
int check_latitude(const struct nmea_angle_t * v);
int check_longitude(const struct nmea_angle_t * v);
const char * parse_int(const char * s, const char * e, uint32_t * val);
const char * parse_float(const char * s, const char * e, float * val);
const char * parse_time(const char * s, const char * e, struct nmea_time_t * v);
const char * parse_date(const char * s, const char * e, struct nmea_date_t * v);
const char * parse_angle(const char * s, const char * e, struct nmea_angle_t * v);

#endif
