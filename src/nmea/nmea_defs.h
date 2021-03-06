#ifndef __NMEA_DEFS__H__
#define __NMEA_DEFS__H__

/* more information at:
 * http://aprs.gids.nl/nmea/
 * http://www.nacs.de/schiffselektronik/nmea-0183.html
 * http://www.nmea.de/nmea0183datensaetze.html
 * http://www.gpsinformation.org/dale/nmea.htm
 */

#include <stdint.h>
#include <nmea/nmea_fix.h>
#include <nmea/nmea_time.h>
#include <nmea/nmea_date.h>
#include <nmea/nmea_angle.h>
#include <nmea/nmea_satelite.h>

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

#define NMEA_UNIT_METER   'M'
#define NMEA_UNIT_FEET    'f'
#define NMEA_UNIT_NM      'N' /* nautical miles */
#define NMEA_UNIT_FATHOM  'F'

#define NMEA_UNIT_KNOT    'N'
#define NMEA_UNIT_KMH     'K' /* kilometers per hour */
#define NMEA_UNIT_MPS     'M' /* meters per second */

#define NMEA_UNIT_CELSIUS 'C'

#define NMEA_TRUE     'T'
#define NMEA_MAGNETIC 'M'
#define NMEA_RELATIVE 'R'

#define NMEA_LEFT  'L'
#define NMEA_RIGHT 'R'

#define NMEA_COMPLETE_ROUTE 'C'
#define NMEA_WORKING_ROUTE  'W'

#define NMEA_SELECTIONMODE_MANUAL    'M'
#define NMEA_SELECTIONMODE_AUTOMATIC 'A'

#define NMEA_NONE       0x00000000
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
#define NMEA_II_MWV     0x00003000 /* integrated instrumentation: wind values */
#define NMEA_II_VWR     0x00003001 /* integrated instrumentation: wind relative */
#define NMEA_II_VWT     0x00003002 /* integrated instrumentation: wind true */
#define NMEA_II_DBT     0x00003003 /* integrated instrumentation: water depth */
#define NMEA_II_VLW     0x00003004 /* integrated instrumentation: log distance */
#define NMEA_II_VHW     0x00003005 /* integrated instrumentation: heading */
#define NMEA_II_MTW     0x00003006 /* integrated instrumentation: temperature water */

/**
 * This structure contains all data provided by the RMB sentence.
 */
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
} __attribute__((packed));

/**
 * This structure contains all data provided by the RMC sentence.
 */
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
} __attribute__((packed));

/**
 * This structure contains all data provided by the GGA sentence.
 */
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
} __attribute__((packed));

/**
 * This structure contains all data provided by the GSA sentence.
 */
struct nmea_gsa_t {
	char selection_mode; /* A:automatic, M:manual */
	uint32_t mode;
	uint32_t id[12];
	struct nmea_fix_t pdop;
	struct nmea_fix_t hdop;
	struct nmea_fix_t vdop;
} __attribute__((packed));

/**
 * This structure contains all data provided by the GSV sentence.
 */
struct nmea_gsv_t {
	uint32_t n_messages;
	uint32_t message_number;
	struct nmea_satelite_t sat[4]; /* max satelites per message */
} __attribute__((packed));

/**
 * This structure contains all data provided by the GLL sentence.
 */
struct nmea_gll_t {
	struct nmea_angle_t lat;
	char lat_dir;
	struct nmea_angle_t lon;
	char lon_dir;
	struct nmea_time_t time; /* utc */
	char status;
} __attribute__((packed));

/**
 * This structure contains all data provided by the RTE sentence.
 */
struct nmea_rte_t {
	uint32_t n_messages;
	uint32_t message_number;
	char message_mode; /* C:complete route, W:working route */
	char waypoint_id[10][8]; /* names or numbers of the active route */
} __attribute__((packed));

/**
 * This structure contains all data provided by the VTG sentence.
 */
struct nmea_vtg_t {
	struct nmea_fix_t track_true;
	char type_true; /* T:true */
	struct nmea_fix_t track_magn;
	char type_magn; /* M:magnetic */
	struct nmea_fix_t speed_kn;
	char unit_speed_kn; /* N:knots */
	struct nmea_fix_t speed_kmh;
	char unit_speed_kmh; /* K:kilometers per hour */
} __attribute__((packed));

/**
 * This structure contains all data provided by the BOD sentence.
 */
struct nmea_bod_t {
	struct nmea_fix_t bearing_true;
	char type_true; /* T:true */
	struct nmea_fix_t bearing_magn;
	char type_magn; /* M:magnetic */
	uint32_t waypoint_to; /* TO waypoint ID */
	uint32_t waypoint_from; /* FROM waypoint ID */
} __attribute__((packed));

/**
 * This structure contains all data provided by the RME sentence.
 */
struct nmea_garmin_rme_t { /* estimated position error */
	struct nmea_fix_t hpe; /* horizontal position error in meters */
	char unit_hpe;
	struct nmea_fix_t vpe; /* vertical position error in meters */
	char unit_vpe;
	struct nmea_fix_t sepe; /* spherical equivalent position error in meters */
	char unit_sepe;
} __attribute__((packed));

/**
 * This structure contains all data provided by the RMM sentence.
 */
struct nmea_garmin_rmm_t { /* map datum */
	char map_datum[64];
} __attribute__((packed));

/**
 * This structure contains all data provided by the RMZ sentence.
 */
struct nmea_garmin_rmz_t { /* altitude information */
	struct nmea_fix_t alt; /* altitude in feet */
	char unit_alt; /* f:feet */
	uint32_t pos_fix_dim; /* 2:user altitude, 3:gps altitude */
} __attribute__((packed));

/**
 * This structure contains all data provided by the HDG sentence.
 */
struct nmea_hc_hdg_t {
	struct nmea_fix_t heading; /* magnetic sensor heading in deg */
	struct nmea_fix_t magn_dev; /* magnetic deviation in deg */
	char magn_dev_dir; /* E:east, W:west */
	struct nmea_fix_t magn_var; /* magnetic variation in deg */
	char magn_var_dir; /* E:east, W:west */
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIMWV sentence.
 */
struct nmea_ii_mwv_t {
	struct nmea_fix_t angle; /* wind angle, 0..359 */
	char type; /* R:relative, T:true */
	struct nmea_fix_t speed; /* wind speed */
	char speed_unit; /* wind speed unit, K:knots, M:mph */
	char status; /* status, A:valid */
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIVWR sentence.
 */
struct nmea_ii_vwr_t {
	struct nmea_fix_t angle; /* wind angle, 0..180 */
	char side; /* side of vessel, R:right, L:left */
	struct nmea_fix_t speed_knots; /* wind speed in knots */
	char speed_knots_unit; /* N:knots */
	struct nmea_fix_t speed_mps; /* wind speed in miles per second */
	char speed_mps_unit; /* M:mps */
	struct nmea_fix_t speed_kmh; /* wind speed in kilometers per hour */
	char speed_kmh_unit; /* K:kmh */
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIVWT sentence.
 */
struct nmea_ii_vwt_t {
	struct nmea_fix_t angle; /* wind angle, 0..180 */
	char side; /* side of vessel, R:right, L:left */
	struct nmea_fix_t speed_knots; /* wind speed in knots */
	char speed_knots_unit; /* N:knots */
	struct nmea_fix_t speed_mps; /* wind speed in miles per second */
	char speed_mps_unit; /* M:mps */
	struct nmea_fix_t speed_kmh; /* wind speed in kilometers per hour */
	char speed_kmh_unit; /* K:kmh */
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIDBT sentence.
 */
struct nmea_ii_dbt_t {
	struct nmea_fix_t depth_feet; /* water depth in feet */
	char depth_unit_feet; /* f:feet */
	struct nmea_fix_t depth_meter; /* water depth in meter */
	char depth_unit_meter; /* M:meter*/
	struct nmea_fix_t depth_fathom; /* water depth in fathom */
	char depth_unit_fathom; /* F:fathom*/
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIVLW sentence.
 */
struct nmea_ii_vlw_t {
	struct nmea_fix_t distance_cum; /* total cumulative distance */
	char distance_cum_unit; /* N:nautical miles */
	struct nmea_fix_t distance_reset; /* distance since reset */
	char distance_reset_unit; /* N:nautical miles */
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIVHW sentence.
 */
struct nmea_ii_vhw_t {
	char heading_empty; /* heading (empty) */
	char degrees_true; /* T:true */
	struct nmea_fix_t heading; /* heading in degrees, 0..359 */
	char degrees_mag; /* M:magnetic */
	struct nmea_fix_t speed_knots; /* speed in knots */
	char speed_knots_unit; /* N:knots */
	struct nmea_fix_t speed_kmh; /* speed in kilometers per hour */
	char speed_kmh_unit; /* K:kmh */
} __attribute__((packed));

/**
 * This structure contains all data provided by the IIMTW sentence.
 */
struct nmea_ii_mtw_t {
	struct nmea_fix_t temperature; /* water temperature */
	char unit; /* unit degrees, C:celcius */
} __attribute__((packed));

#endif
