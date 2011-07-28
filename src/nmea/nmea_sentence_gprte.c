#include <nmea/nmea_sentence_gprte.h>
#include <nmea/nmea_util.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_rte_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_RTE;
	v = &nmea->sentence.rte;
	p = find_token_end(s);
	for (state = -1; state < 13 && s < e; ++state) {
		switch (state) {
			case  0: if (parse_int(s, p, &v->n_messages) != p) return -1; break;
			case  1: if (parse_int(s, p, &v->message_number) != p) return -1; break;
			case  2: v->message_mode = (s == p) ? NMEA_COMPLETE_ROUTE : *s; break;
			case  3:
			case  4:
			case  5:
			case  6:
			case  7:
			case  8:
			case  9:
			case 10:
			case 11:
			case 12: if ((unsigned int)(p-s+1) < sizeof(v->waypoint_id[state-3]) && parse_str(s, p, v->waypoint_id[state-3]) != p) return -1; break;
			default: break;
		}
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_gprte =
{
	.type = NMEA_RTE,
	.tag = NMEA_SENTENCE_GPRTE,
	.read = read,
	.write = NULL,
};

