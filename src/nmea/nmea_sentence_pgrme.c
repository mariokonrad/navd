#include <nmea/nmea_sentence_pgrme.h>
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
	struct nmea_garmin_rme_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GARMIN_RME;
	v = &nmea->sentence.garmin_rme;
	p = find_token_end(s);
	for (state = -1; state < 6 && s < e; ++state) {
		switch (state) {
			case 0: if (nmea_fix_parse(s, p, &v->hpe) != p) return -1; break;
			case 1: v->unit_hpe = (s == p) ? NMEA_UNIT_METER : *s; break;
			case 2: if (nmea_fix_parse(s, p, &v->vpe) != p) return -1; break;
			case 3: v->unit_vpe = (s == p) ? NMEA_UNIT_METER : *s; break;
			case 4: if (nmea_fix_parse(s, p, &v->sepe) != p) return -1; break;
			case 5: v->unit_sepe= (s == p) ? NMEA_UNIT_METER : *s; break;
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
const struct nmea_sentence_t sentence_pgrme =
{
	.type = NMEA_GARMIN_RME,
	.tag = "PGRME",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

