#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <nmea.h>

#define UNUSED_ARG(a)  (void)a

struct device_t
{
	int fd;
	void * data;
};

struct device_config_t {};

struct device_operations_t
{
	int (*open)(struct device_t *, const struct device_config_t *);
	int (*close)(struct device_t *);
	int (*read)(struct device_t *, char *, size_t);
	int (*write)(struct device_t *, const char *, size_t);
};

static void device_init(struct device_t * device)
{
	if (device == NULL) return;
	device->fd = -1;
	device->data = NULL;
}

/* {{{ simulator */

static struct simulator_data_t
{
	const char * s;
	int i;
	int len;
} simulator_data;

static int simulator_open(struct device_t * device, const struct device_config_t * cfg)
{
	UNUSED_ARG(cfg);

	if (device == NULL) return -1;
	if (device->fd >= 0) return 0;
	device->fd = 0;
	device->data = &simulator_data;
	simulator_data.i = 0;
	simulator_data.s =
		"$GPRMC,202451,A,4702.3966,N,00818.3287,E,0.0,312.3,260711,0.6,E,A*19\r\n"
		"$GPGSV,3,1,12,03,77,155,29,06,65,131,34,07,05,288,00,08,08,314,00*76\r\n"
		"$GPGLL,4702.3966,N,00818.3287,E,202451,A,A*43\r\n"
		"\0"
		;
	simulator_data.len = strlen(simulator_data.s);
	return 0;
}

static int simulator_close(struct device_t * device)
{
	if (device == NULL) return -1;
	if (device->fd < 0) return 0;
	device->fd = 0;
	return 0;
}

static int simulator_read(struct device_t * device, char * buf, size_t size)
{
	struct simulator_data_t * data = NULL;
	size_t i;

	if (device == NULL) return -1;
	if (buf == NULL) return -1;
	if (device->fd < 0) return -1;

	data = (struct simulator_data_t *)device->data;
	for (i = 0; i < size; ++i) {
		buf[i] = data->s[data->i];
		data->i = (data->i + 1) % data->len;
	}
	return (int)size;
}

static int simulator_write(struct device_t * device, const char * buf, size_t size)
{
	UNUSED_ARG(device);
	UNUSED_ARG(buf);
	UNUSED_ARG(size);
	return -1;
}

const struct device_operations_t simulator_operations =
{
	.open = simulator_open,
	.close = simulator_close,
	.read = simulator_read,
	.write = simulator_write,
};

/* }}} */

/* {{{ serial */

struct serial_config_t
{
	char name[128];
};

static int serial_open(struct device_t * device, const struct device_config_t * cfg)
{
	struct termios old_tio;
	struct termios new_tio;
	const struct serial_config_t * config = NULL;

	if (device == NULL) return -1;
	if (cfg == NULL) return -1;
	if (device->fd >= 0) return 0;

	config = (const struct serial_config_t *)cfg;

	device->fd = open(config->name, O_RDWR | O_NOCTTY);
	if (device->fd < 0) return -1;

	tcgetattr(device->fd, &old_tio);
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

	tcflush(device->fd, TCIFLUSH);	
	tcsetattr(device->fd, TCSANOW, &new_tio);
	return 0;
}

static int serial_close(struct device_t * device)
{
	if (device == NULL) return -1;
	if (device->fd < 0) return 0;
	close(device->fd);
	device->fd = -1;
	return 0;
}

static int serial_read(struct device_t * device, char * buf, size_t size)
{
	if (device == NULL) return -1;
	if (buf == NULL) return -1;
	if (device->fd < 0) return -1;
	return read(device->fd, buf, size);
}

static int serial_write(struct device_t * device, const char * buf, size_t size)
{
	if (device == NULL) return -1;
	if (buf == NULL) return -1;
	if (device->fd < 0) return -1;
	return write(device->fd, buf, size);
}

const struct device_operations_t serial_device_operations =
{
	.open = serial_open,
	.close = serial_close,
	.read = serial_read,
	.write = serial_write,
};

/* }}} */

/* TODO {{{ ethernet-tcp */
/* }}} */

/* TODO {{{ ethernet-udp */
/* }}} */

int main(int argc, char ** argv)
{
	int rc;
	char c;
	int i = 0;
	char s[MAX_NMEA_SENTENCE + 1];
	struct nmea_t nmea;
	int num_sentences = 4;
	int type = 0;

	struct device_t device;
	const struct device_operations_t * ops = NULL;
	const struct device_config_t * config = NULL;

	struct serial_config_t serial_config = {
		"/dev/ttyUSB0"
	};

	memset(s, 0, sizeof(s));
	device_init(&device);

	switch (type) {
		case 0: {
			config = NULL;
			ops = &simulator_operations;
			} break;
		case 1: {
			config = &serial_config;
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
				if (i < MAX_NMEA_SENTENCE) {
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

