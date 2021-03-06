#include <nmea/nmea_sentence_gpbod.h>
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
	struct nmea_bod_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_BOD;
	v = &nmea->sentence.bod;
	p = find_token_end(s);
	for (state = -1; state < 6; ++state) {
		switch (state) {
			case  0: if (nmea_fix_parse(s, p, &v->bearing_true) != p) return -1; break;
			case  1: v->type_true = (s == p) ? NMEA_TRUE : *s; break;
			case  2: if (nmea_fix_parse(s, p, &v->bearing_magn) != p) return -1; break;
			case  3: v->type_magn = (s == p) ? NMEA_MAGNETIC : *s; break;
			case  4: if (parse_int(s, p, &v->waypoint_to) != p) return -1; break;
			case  5: if (parse_int(s, p, &v->waypoint_from) != p) return -1; break;
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
const struct nmea_sentence_t sentence_gpbod =
{
	.type = NMEA_BOD,
	.tag = "GPBOD",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

