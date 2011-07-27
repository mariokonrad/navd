#include <nmea_sentence_hchdg.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_hchdg(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_hc_hdg_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_HC_HDG;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.hc_hdg;
	switch (state) {
		case 0: if (parse_fix(s, p, &v->heading) != p) return -1; break;
		case 1: if (parse_fix(s, p, &v->magn_dev) != p) return -1; break;
		case 2: v->magn_dev_dir = (s == p) ? NMEA_EAST : *s; break;
		case 3: if (parse_fix(s, p, &v->magn_var) != p) return -1; break;
		case 4: v->magn_dev_dir = (s == p) ? NMEA_EAST : *s; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_hchdg =
{
	.type = NMEA_HC_HDG,
	.tag = NMEA_SENTENCE_HCHDG,
	.parse = sentence_parser_hchdg,
	.write = NULL,
};

