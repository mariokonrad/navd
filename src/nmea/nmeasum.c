#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <nmea/nmea_checksum.h>

int main(int argc, char ** argv)
{
	uint8_t sum = 0;

	if (argc != 2) {
		printf("usage: %s \"sentence\" (sentence without start/end token)\n", argv[0]);
		return EXIT_FAILURE;
	}

	sum = nmea_checksum(argv[1], argv[1] + strlen(argv[1]));
	printf("%02X : '%s'\n", sum, argv[1]);

	return EXIT_SUCCESS;
}

