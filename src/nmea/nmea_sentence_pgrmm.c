#include <nmea/nmea_sentence_pgrmm.h>
#include <nmea/nmea_util.h>
#include <stdio.h>

/**
 * Reads the NMEA sentence into the specified structure.
 *
 * @param[out] nmea Structure to hold the parsed data.
 * @param[in] s Start of the string to parse (inclusive).
 * @param[in] e End of the string to parse (exclusive).
 * @retval -1 Parameter failure, parsing error.
 * @retval  0 Success
 */
static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_garmin_rmm_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GARMIN_RMM;
	v = &nmea->sentence.garmin_rmm;
	p = find_token_end(s);
	for (state = -1; state < 1 && s < e; ++state) {
		switch (state) {
			case 0: if ((unsigned int)(p-s+1) < sizeof(v->map_datum) && parse_str(s, p, v->map_datum) != p) return -1; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

/**
 * Description of the NMEA sentence.
 */
const struct nmea_sentence_t sentence_pgrmm =
{
	.type = NMEA_GARMIN_RMM,
	.tag = "PGRMM",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

