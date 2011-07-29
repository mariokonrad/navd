#include <nmea/nmea_sentence_gpgsv.h>
#include <nmea/nmea_util.h>
#include <nmea/nmea_int.h>
#include <stdio.h>

static int read(struct nmea_t * nmea, const char * s, const char * e)
{
	struct nmea_gsv_t * v;
	int state = 0;
	const char * p;

	if (nmea == NULL || s == NULL || e == NULL) return -1;
	nmea->type = NMEA_GSV;
	v = &nmea->sentence.gsv;
	p = find_token_end(s);
	for (state = -1; state < 18 && s < e; ++state) {
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
		s = p + 1;
		p = find_token_end(s);
	}
	return 0;
}

const struct nmea_sentence_t sentence_gpgsv =
{
	.type = NMEA_GSV,
	.tag = "GPGSV",
	.read = read,
	.write = NULL,
	.hton = NULL,
	.ntoh = NULL,
};

