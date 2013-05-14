#include <nmea/nmea_sentence_iimtw.h>
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
	struct nmea_ii_mtw_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_II_MTW;
	v = &nmea->sentence.ii_mtw;
	p = find_token_end(s);
	for (state = -1; state < 2 && s < e; ++state) {
		switch (state) {
			case  0: if (nmea_fix_parse(s, p, &v->temperature) != p) return -1; break;
			case  1: v->unit = (s == p) ? NMEA_UNIT_CELSIUS : *s; break;
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
const struct nmea_sentence_t sentence_iimtw =
{
	.type = NMEA_II_MTW,
	.tag = "IIMTW",
	.read= read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

