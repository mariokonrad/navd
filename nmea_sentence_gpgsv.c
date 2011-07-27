#include <nmea_sentence_gpgsv.h>
#include <nmea_util.h>
#include <stdio.h>

static int sentence_parser_gpgsv(int state, const char * s, const char * p, struct nmea_t * nmea)
{
	struct nmea_gsv_t * v;
	if (nmea == NULL) return -1;
	if (state == -1) {
		nmea->type = NMEA_GSV;
		return 0;
	}
	if (s == NULL || p == NULL) return -1;
	v = &nmea->sentence.gsv;
	switch (state) {
		case  0: if (parse_int(s, p, &v->n_messages) != p) return -1; break;
		case  1: if (parse_int(s, p, &v->message_number) != p) return -1; break;
		case  2: if (parse_int(s, p, &v->sat[0].id) != p) return -1; break;
		case  3: if (parse_int(s, p, &v->sat[0].elevation) != p) return -1; break;
		case  4: if (parse_int(s, p, &v->sat[0].azimuth) != p) return -1; break;
		case  5: if (parse_int(s, p, &v->sat[0].snr) != p) return -1; break;
		case  6: if (parse_int(s, p, &v->sat[1].id) != p) return -1; break;
		case  7: if (parse_int(s, p, &v->sat[1].elevation) != p) return -1; break;
		case  8: if (parse_int(s, p, &v->sat[1].azimuth) != p) return -1; break;
		case  9: if (parse_int(s, p, &v->sat[1].snr) != p) return -1; break;
		case 10: if (parse_int(s, p, &v->sat[2].id) != p) return -1; break;
		case 11: if (parse_int(s, p, &v->sat[2].elevation) != p) return -1; break;
		case 12: if (parse_int(s, p, &v->sat[2].azimuth) != p) return -1; break;
		case 13: if (parse_int(s, p, &v->sat[2].snr) != p) return -1; break;
		case 14: if (parse_int(s, p, &v->sat[3].id) != p) return -1; break;
		case 15: if (parse_int(s, p, &v->sat[3].elevation) != p) return -1; break;
		case 16: if (parse_int(s, p, &v->sat[3].azimuth) != p) return -1; break;
		case 17: if (parse_int(s, p, &v->sat[3].snr) != p) return -1; break;
		default: break;
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgsv =
{
	.type = NMEA_GSV,
	.tag = NMEA_SENTENCE_GPGSV,
	.parse = sentence_parser_gpgsv,
	.write = NULL,
};

