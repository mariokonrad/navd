#ifndef __NMEA_BASE__H__
#define __NMEA_BASE__H__

#include <stdint.h>
#include <nmea/nmea_defs.h>

#define START_TOKEN_NMEA '$'
#define START_TOKEN_AIVDM '!'

#define NMEA_MAX_SENTENCE 82

/**
 * Represents a NMEA message, containing the original raw NMEA sentence
 * and the already (if possible) parsed data.
 */
struct nmea_t {
	uint32_t type;
	char raw[NMEA_MAX_SENTENCE+1];
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
		struct nema_ii_mwv_t ii_mwv;
		struct nmea_ii_vwr_t ii_vwr;
		struct nmea_ii_vwr_t ii_vwt;
		struct nmea_ii_dbt_t ii_dbt;
		struct nmea_ii_vlw_t ii_vlw;
		struct nmea_ii_vhw_t ii_vhw;
		struct nmea_ii_mtw_t ii_mtw;
	} sentence;
};

/**
 * Base structure for all implmentations of NMEA sentences.
 */
struct nmea_sentence_t {
	const uint32_t type;
	const char * tag;
	int (*read)(struct nmea_t *, const char *, const char *);
	int (*write)(char *, uint32_t, const struct nmea_t *);
	void (*hton)(struct nmea_t *);
	void (*ntoh)(struct nmea_t *);
};

int nmea_init(struct nmea_t *);

int nmea_read_tab(struct nmea_t *, const char *, const struct nmea_sentence_t **, uint32_t);

int nmea_write_tab(char *, uint32_t, const struct nmea_t *, const struct nmea_sentence_t **, uint32_t);
int nmea_write_raw(char *, uint32_t, const struct nmea_t *);

int nmea_hton_tab(struct nmea_t *, const struct nmea_sentence_t **, uint32_t);
int nmea_ntoh_tab(struct nmea_t *, const struct nmea_sentence_t **, uint32_t);

#endif
