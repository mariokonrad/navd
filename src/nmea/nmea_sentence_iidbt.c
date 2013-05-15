#include <nmea/nmea_sentence_iidbt.h>
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
	struct nmea_ii_dbt_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_II_DBT;
	v = &nmea->sentence.ii_dbt;
	p = find_token_end(s);
	for (state = -1; state < 6 && s < e; ++state) {
		switch (state) {
			case  0: if (nmea_fix_parse(s, p, &v->depth_feet) != p) return -1; break;
			case  1: v->depth_unit_feet = (s == p) ? NMEA_UNIT_FEET : *s; break;
			case  2: if (nmea_fix_parse(s, p, &v->depth_meter) != p) return -1; break;
			case  3: v->depth_unit_meter = (s == p) ? NMEA_UNIT_METER : *s; break;
			case  4: if (nmea_fix_parse(s, p, &v->depth_fathom) != p) return -1; break;
			case  5: v->depth_unit_fathom = (s == p) ? NMEA_UNIT_FATHOM : *s; break;
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
const struct nmea_sentence_t sentence_iidbt =
{
	.type = NMEA_II_DBT,
	.tag = "IIDBT",
	.read= read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

