#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <nmea/nmea.h>
#include <device/simulator.h>
#include <device/serial.h>

int main(int argc, char ** argv)
{
	int rc;
	char c;
	int i = 0;
	char s[NMEA_MAX_SENTENCE + 1];
	struct nmea_t nmea;
	int num_sentences = 4;
	int type = 0;

	struct device_t device;
	const struct device_operations_t * ops = NULL;
	const struct device_config_t * config = NULL;

	struct serial_config_t serial_config = {
		"/dev/ttyUSB0"
	};

	UNUSED_ARG(argc);
	UNUSED_ARG(argv);

	memset(s, 0, sizeof(s));
	nmea_init(&nmea);
	device_init(&device);

	switch (type) {
		case 0: {
			config = NULL;
			ops = &simulator_operations;
			} break;
		case 1: {
			config = (const struct device_config_t *)&serial_config;
			ops = &serial_device_operations;
			} break;
	}

	rc = ops->open(&device, config);
	if (rc < 0) {
		perror("open");
		return -1;
	}

	while (num_sentences > 0) {
		rc = ops->read(&device, &c, sizeof(c));
		if (rc < 0) {
			perror("read");
			break;
		}
		if (rc != sizeof(c)) {
			perror("read");
			break;
		}
		switch (c) {
			case '\r':
				break;
			case '\n':
				rc = nmea_read(&nmea, s);
				if (rc == 0) {
					printf("OK : [%s]\n", s);
				} else if (rc == 1) {
					printf("[%s] : UNKNOWN SENTENCE\n", s);
				} else if (rc == -2) {
					printf("[%s] : CHECKSUM ERROR\n", s);
				} else {
					fprintf(stderr, "parameter error\n");
					return -1;
				}
				i = 0;
				s[i] = 0;
				--num_sentences;
				break;
			default:
				if (i < NMEA_MAX_SENTENCE) {
					s[i++] = c;
				} else {
					fprintf(stderr, "sentence too long, discarding\n");
					i = 0;
				}
				s[i] = 0;
				break;
		}
	}

	rc = ops->close(&device);
	if (rc < 0) {
		perror("close");
		return -1;
	}

	return 0;
}

