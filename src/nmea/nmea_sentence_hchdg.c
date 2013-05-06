#include <nmea/nmea_sentence_hchdg.h>
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
	struct nmea_hc_hdg_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_HC_HDG;
	v = &nmea->sentence.hc_hdg;
	p = find_token_end(s);
	for (state = -1; state < 5 && s < e; ++state) {
		switch (state) {
			case 0: if (nmea_fix_parse(s, p, &v->heading) != p) return -1; break;
			case 1: if (nmea_fix_parse(s, p, &v->magn_dev) != p) return -1; break;
			case 2: v->magn_dev_dir = (s == p) ? NMEA_EAST : *s; break;
			case 3: if (nmea_fix_parse(s, p, &v->magn_var) != p) return -1; break;
			case 4: v->magn_dev_dir = (s == p) ? NMEA_EAST : *s; break;
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
const struct nmea_sentence_t sentence_hchdg =
{
	.type = NMEA_HC_HDG,
	.tag = "HCHDG",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

