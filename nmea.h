#ifndef __NMEA__H__
#define __NMEA__H__

#include <stdint.h>

#define NMEA_RMA 0x0001
#define NMEA_RMB 0x0002
#define NMEA_RMC 0x0003
#define NMEA_GGA 0x0004
#define NMEA_GSA 0x0005
#define NMEA_GSV 0x0006

struct nmea_rma_t {
	/* TODO */
};

struct nmea_rmb_t {
	/* TODO */
};

struct nmea_rmc_t {
	/* TODO */
};

struct nmea_gga_t {
	/* TODO */
};

struct nmea_gsa_t {
	/* TODO */
};

struct nmea_gsv_t {
	/* TODO */
};

struct nmea_t {
	uint16_t type;
	union {
		struct nmea_rma_t rma;
		struct nmea_rmb_t rmb;
		struct nmea_rmc_t rmc;
		struct nmea_gga_t gga;
		struct nmea_gsa_t gsa;
		struct nmea_gsv_t gsv;
	} sentence;
};

int nmea_read(const char *, struct nmea_t *);

#endif
