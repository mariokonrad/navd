#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int fd;
	struct termios old_tio;
	struct termios new_tio;
	int rc;
	int n;
	char c;

	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if (fd < 0)
		return -1;

	tcgetattr(fd, &old_tio);
	memset(&new_tio, 0, sizeof(new_tio));
	new_tio.c_cflag = 0
		| B4800
		| CS8
		| CLOCAL
		| CREAD
		;
	new_tio.c_iflag = 0
		| IGNPAR
		;
	new_tio.c_oflag = 0
		;
	new_tio.c_lflag = 0
		;

	new_tio.c_cc[VMIN]  = 1;
	new_tio.c_cc[VTIME] = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &new_tio);

	n = 0;
	for (;;) {
		rc = read(fd, &c, sizeof(c));
		if (rc < 0) {
			perror("read");
			return -1;
		}
		if (n % 16 == 0) {
			n = 0;
			printf("\n");
		}
		printf(" 0x%02x", (int)(c & 0xff));
		fflush(stdout);
		++n;
	}

	close(fd);
	return 0;
}

