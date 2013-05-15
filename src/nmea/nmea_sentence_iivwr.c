#include <nmea/nmea_sentence_iivwr.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_int.h>
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
	struct nmea_ii_vwr_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_II_VWR;
	v = &nmea->sentence.ii_vwr;
	p = find_token_end(s);
	for (state = -1; state < 8 && s < e; ++state) {
		switch (state) {
			case  0: if (nmea_fix_parse(s, p, &v->angle) != p) return -1; break;
			case  1: v->side = (s == p) ? NMEA_RIGHT : *s; break;
			case  2: if (nmea_fix_parse(s, p, &v->speed_knots) != p) return -1; break;
			case  3: v->speed_knots_unit = (s == p) ? NMEA_UNIT_KNOT : *s; break;
			case  4: if (nmea_fix_parse(s, p, &v->speed_mps) != p) return -1; break;
			case  5: v->speed_mps_unit = (s == p) ? NMEA_UNIT_MPS : *s; break;
			case  6: if (nmea_fix_parse(s, p, &v->speed_kmh) != p) return -1; break;
			case  7: v->speed_kmh_unit = (s == p) ? NMEA_UNIT_KMH : *s; break;
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
const struct nmea_sentence_t sentence_iivwr =
{
	.type = NMEA_II_VWR,
	.tag = "IIVWR",
	.read= read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

