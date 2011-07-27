#ifndef __NMEA__H__
#define __NMEA__H__

/* more information at:
 * http://aprs.gids.nl/nmea/
 * http://www.nacs.de/schiffselektronik/nmea-0183.html
 * http://www.nmea.de/nmea0183datensaetze.html
 * http://www.gpsinformation.org/dale/nmea.htm
 */

#include <stdint.h>

#define MAX_NMEA_SENTENCE 82

#define NMEA_SENTENCE_GPRMB "GPRMB"
#define NMEA_SENTENCE_GPRMC "GPRMC"
#define NMEA_SENTENCE_GPGGA "GPGGA"
#define NMEA_SENTENCE_GPGSV "GPGSV"
#define NMEA_SENTENCE_GPGSA "GPGSA"
#define NMEA_SENTENCE_GPGLL "GPGLL"
#define NMEA_SENTENCE_GPBOD "GPBOD"
#define NMEA_SENTENCE_GPVTG "GPVTG"
#define NMEA_SENTENCE_GPRTE "GPRTE"
#define NMEA_SENTENCE_PGRME "PGRME"
#define NMEA_SENTENCE_PGRMM "PGRMM"
#define NMEA_SENTENCE_PGRMZ "PGRMZ"
#define NMEA_SENTENCE_HCHDG "HCHDG"

#define NMEA_NONE       0x00000000
#define NMEA_RMA        0x00000001
#define NMEA_RMB        0x00000002
#define NMEA_RMC        0x00000003
#define NMEA_GGA        0x00000004
#define NMEA_GSA        0x00000005
#define NMEA_GSV        0x00000006
#define NMEA_GLL        0x00000007
#define NMEA_RTE        0x00000008
#define NMEA_VTG        0x00000009
#define NMEA_BOD        0x0000000a
#define NMEA_GARMIN_RME 0x00001000
#define NMEA_GARMIN_RMM 0x00001001
#define NMEA_GARMIN_RMZ 0x00001002
#define NMEA_HC_HDG     0x00002000

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
#define NMEA_UNIT_FEET  'f'
#define NMEA_UNIT_KNOT  'N'
#define NMEA_UNIT_KMH   'K'

#define NMEA_TRUE     'T'
#define NMEA_MAGNETIC 'M'

#define NMEA_LEFT  'L'
#define NMEA_RIGHT 'R'

#define NMEA_COMPLETE_ROUTE 'C'
#define NMEA_WORKING_ROUTE  'W'

#define NMEA_SELECTIONMODE_MANUAL    'M'
#define NMEA_SELECTIONMODE_AUTOMATIC 'A'

#define NMEA_FIX_DECIMALS 1000000
#define NMEA_FIX_DECIMAL_DIGITS 6
#define NMEA_ANGLE_DECIMALS 10000
#define NMEA_ANGLE_DECIMAL_DIGITS 4

#define START_TOKEN_NMEA '$'
#define START_TOKEN_AIVDM '!'

struct nmea_fix_t { /* x.xxxxxx */
	uint32_t i; /* integer part, max. 6 digits, see NMEA_FIX_DECIMALS */
	uint32_t d; /* decimal part, 6 digits, see NMEA_FIX_DECIMALS */
};

struct nmea_time_t {
	uint32_t h;  /* hour: 0..23 */
	uint32_t m;  /* minute: 0..59 */
	uint32_t s;  /* second: 0..59 */
	uint32_t ms; /* millisecond: 0..999 */
};

struct nmea_date_t {
	uint32_t y; /* year */
	uint32_t m; /* month: 1..12 */
	uint32_t d; /* day: 1..31 */
};

struct nmea_angle_t {
	uint32_t d; /* degrees */
	uint32_t m; /* minutes */
	struct nmea_fix_t s; /* seconds */
};

struct nmea_satelite_t {
	uint32_t id;
	uint32_t elevation;
	uint32_t azimuth; /* azimuth against true */
	uint32_t snr;
};

struct nmea_rmb_t {
	char status; /* V:warning */
	struct nmea_fix_t cross_track_error; /* cross track error in nautical miles */
	char steer_dir; /* direction to steer, left or right */
	uint32_t waypoint_to; /* TO waypoint ID */
	uint32_t waypoint_from; /* FROM waypoint ID */
	struct nmea_angle_t lat; /* destination waypoint latitude */
	char lat_dir; /* destination waypoint latitude dir, N:north, S:south */
	struct nmea_angle_t lon; /* destination waypoint longitude */
	char lon_dir; /* destination waypoint longitude dir, E:east, W:west */
	struct nmea_fix_t range; /* range to destination in nautical miles */
	struct nmea_fix_t bearing; /* bearing to destination in degrees to true */
	struct nmea_fix_t dst_velocity; /* destination closing velocity in knots */
	char arrival_status; /* arrival status, A:arrival circle entered */
};

struct nmea_rmc_t {
	struct nmea_time_t time;
	char status; /* A:ok, V:warning */
	struct nmea_angle_t lat;
	char lat_dir; /* N:north, S:south */
	struct nmea_angle_t lon;
	char lon_dir; /* E:east, W:west */
	struct nmea_fix_t sog; /* speed over ground in knots */
	struct nmea_fix_t head; /* heading over ground in degrees regarding geograpic north */
	struct nmea_date_t date;
	struct nmea_fix_t m; /* magnetic deviation */
	char m_dir; /* magnetic deviation E:east, W:west */
	char sig_integrity; /* signal integrity mode, A:autonomous, D:differential, E:estimated, M:manual input, S:simulated, N:data not valid */
};

struct nmea_gga_t {
	struct nmea_time_t time;
	struct nmea_angle_t lat;
	char lat_dir; /* N:north, S:south */
	struct nmea_angle_t lon;
	char lon_dir; /* E:east, W:west */
	uint32_t quality;
	uint32_t n_satelites;
	struct nmea_fix_t hor_dilution; /* horizontal dilution of precision */
	struct nmea_fix_t height_antenna; /* height of antenna over geoid */
	char unit_antenna; /* M:meter */
	struct nmea_fix_t geodial_separation; /* geodial separation, sea level below the ellipsoid */
	char unit_geodial_separation; /* M:meter */
	struct nmea_fix_t dgps_age; /* age of dgps data */
	uint32_t dgps_ref; /* dgps reference station 0000..1023 */
};

struct nmea_gsa_t {
	char selection_mode; /* A:automatic, M:manual */
	uint32_t mode;
	uint32_t id[12];
	struct nmea_fix_t pdop;
	struct nmea_fix_t hdop;
	struct nmea_fix_t vdop;
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

struct nmea_rte_t {
	uint32_t n_messages;
	uint32_t message_number;
	char message_mode; /* C:complete route, W:working route */
	char waypoint_id[10][8]; /* names or numbers of the active route */
};

struct nmea_vtg_t {
	struct nmea_fix_t track_true;
	char type_true; /* T:true */
	struct nmea_fix_t track_magn;
	char type_magn; /* M:magnetic */
	struct nmea_fix_t speed_kn;
	char unit_speed_kn; /* N:knots */
	struct nmea_fix_t speed_kmh;
	char unit_speed_kmh; /* K:kilometers per hour */
};

struct nmea_bod_t {
	struct nmea_fix_t bearing_true;
	char type_true; /* T:true */
	struct nmea_fix_t bearing_magn;
	char type_magn; /* M:magnetic */
	uint32_t waypoint_to; /* TO waypoint ID */
	uint32_t waypoint_from; /* FROM waypoint ID */
};

struct nmea_garmin_rme_t { /* estimated position error */
	struct nmea_fix_t hpe; /* horizontal position error in meters */
	char unit_hpe;
	struct nmea_fix_t vpe; /* vertical position error in meters */
	char unit_vpe;
	struct nmea_fix_t sepe; /* spherical equivalent position error in meters */
	char unit_sepe;
};

struct nmea_garmin_rmm_t { /* map datum */
	char map_datum[64];
};

struct nmea_garmin_rmz_t { /* altitude information */
	struct nmea_fix_t alt; /* altitude in feet */
	char unit_alt; /* f:feet */
	uint32_t pos_fix_dim; /* 2:user altitude, 3:gps altitude */
};

struct nmea_hc_hdg_t {
	struct nmea_fix_t heading; /* magnetic sensor heading in deg */
	struct nmea_fix_t magn_dev; /* magnetic deviation in deg */
	char magn_dev_dir; /* E:east, W:west */
	struct nmea_fix_t magn_var; /* magnetic variation in deg */
	char magn_var_dir; /* E:east, W:west */
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
		struct nmea_bod_t bod;
		struct nmea_vtg_t vtg;
		struct nmea_rte_t rte;
		struct nmea_garmin_rme_t garmin_rme;
		struct nmea_garmin_rmm_t garmin_rmm;
		struct nmea_garmin_rmz_t garmin_rmz;
		struct nmea_hc_hdg_t hc_hdg;
	} sentence;
};

struct nmea_sentence_t {
	const uint32_t type;
	const char * tag;
	int (*read)(struct nmea_t *, const char *, const char *);
	int (*write)(char *, uint32_t, const struct nmea_t *);
};

int nmea_read_tab(struct nmea_t *, const char *, const struct nmea_sentence_t **, uint32_t);
int nmea_read(struct nmea_t *, const char *);

int nmea_write_tab(char *, uint32_t, const struct nmea_t *, const struct nmea_sentence_t **, uint32_t);
int nmea_write(char *, uint32_t, const struct nmea_t *);

#endif
