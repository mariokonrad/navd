#include <stdio.h>
#include <navcom/message.h>
#include <device/device.h>
#include <device/serial.h>
#include <nmea/nmea.h>
#include <common/macros.h>

#define print(t) printf("%30s : %lu\n", #t, sizeof(t))

int main(int argc, char ** argv)
{
	UNUSED_ARG(argc);
	UNUSED_ARG(argv);

	print(struct message_t);
	print(struct device_t);
	print(struct serial_config_t);
	print(struct nmea_t);
	print(struct nmea_sentence_t);
	print(struct nmea_rmb_t);
	print(struct nmea_rmc_t);
	print(struct nmea_gga_t);
	print(struct nmea_gsa_t);
	print(struct nmea_gsv_t);
	print(struct nmea_gll_t);
	print(struct nmea_rte_t);
	print(struct nmea_vtg_t);
	print(struct nmea_bod_t);
	print(struct nmea_garmin_rme_t);
	print(struct nmea_garmin_rmm_t);
	print(struct nmea_garmin_rmz_t);
	print(struct nmea_hc_hdg_t);
	print(struct nmea_fix_t);
	print(struct nmea_time_t);
	print(struct nmea_date_t);
	print(struct nmea_satelite_t);
	print(struct nmea_angle_t);

	return 0;
}

