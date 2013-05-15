#include <nmea/nmea_sentence_iivhw.h>
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
	struct nmea_ii_vhw_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_II_VHW;
	v = &nmea->sentence.ii_vhw;
	p = find_token_end(s);
	for (state = -1; state < 8 && s < e; ++state) {
		switch (state) {
			case  0: break; /* ignore emtpy */
			case  1: v->degrees_true = (s == p) ? NMEA_TRUE : *s; break;
			case  2: if (nmea_fix_parse(s, p, &v->heading) != p) return -1; break;
			case  3: v->degrees_mag = (s == p) ? NMEA_MAGNETIC : *s; break;
			case  4: if (nmea_fix_parse(s, p, &v->speed_knots) != p) return -1; break;
			case  5: v->speed_knots_unit = (s == p) ? NMEA_UNIT_KNOT : *s; break;
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
const struct nmea_sentence_t sentence_iivhw =
{
	.type = NMEA_II_VHW,
	.tag = "IIVHW",
	.read= read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

