#include <device/simulator_serial_seatalk.h>
#include <common/macros.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

static const uint8_t DATA[] =
{
	/* preliminary garbage */
	0x01,             /* bit=0 parity=0 : no error : ?    */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : ?    */
	0x01,             /* bit=0 parity=0 : no error : ?    */
	0x01,             /* bit=0 parity=0 : no error : ?    */
	0xff, 0xff,       /* bit=0 parity=1 : error    : ?    */
	0xff, 0xff,       /* bit=0 parity=1 : error    : ?    */
	0x01,             /* bit=0 parity=0 : no error : ?    */

	/* depth */
	0x00,             /* bit=1 parity=1 : no error : cmd  */
	0x02,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x60, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x65, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* speed through water */
	0xff, 0x00, 0x26, /* bit=1 parity=0 : error    : cmd  */
	0x04,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* water temperature */
	0x27,             /* bit=1 parity=1 : no error : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0x64,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* apparent wind speed */
	0x11,             /* bit=1 parity=1 : no error : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x06, /* bit=0 parity=1 : error    : data */
	0x01,             /* bit=0 parity=0 : no error : data */

	/* speed through water */
	0xff, 0x00, 0x20, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* water temperature */
	0xff, 0x00, 0x23, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x33, /* bit=0 parity=1 : error    : data */
	0x5b,             /* bit=0 parity=0 : no error : data */

	/* apparent wind angle */
	0xff, 0x00, 0x10, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x14, /* bit=0 parity=1 : error    : data */

	/* depth */
	0x00,             /* bit=1 parity=1 : no error : cmd  */
	0x02,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x60, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x65, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */

	/* depth, collision */
	0x00,             /* bit=1 parity=1 : no error : cmd  */
	0x02,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x60, /* bit=0 parity=1 : error    : data */

	/* apparent wind angle */
	0xff, 0x00, 0x10, /* bit=1 parity=0 : error    : cmd  */
	0x01,             /* bit=0 parity=0 : no error : data */
	0xff, 0x00, 0x00, /* bit=0 parity=1 : error    : data */
	0xff, 0x00, 0x14, /* bit=0 parity=1 : error    : data */
};
/**
 * Structure to hand simulation data.
 */
struct simulator_data_t
{
	const uint8_t * data;
	unsigned int index;
	unsigned int len;

	int fd;
};

/**
 * Simulator device data. Must be module global in order to
 * grant access to the signal handler for SIGALRM.
 */
static struct simulator_data_t simulator_data = { NULL, 0, 0, -1 };

/**
 * Alarm handler, sends data to the pipe.
 */
static void alarm_handler(int sig, siginfo_t * siginfo, void * context)
{
	uint8_t c;
	int rc;

	UNUSED_ARG(sig);
	UNUSED_ARG(siginfo);
	UNUSED_ARG(context);

	if (simulator_data.fd < 0)
		return;

	c = simulator_data.data[simulator_data.index];
	rc = write(simulator_data.fd, &c, sizeof(c));
	if (rc == sizeof(c)) {
		simulator_data.index = (simulator_data.index + 1) % simulator_data.len;
	}
}

/**
 * Opens the simulator device.
 *
 * @param[out] device The device descriptor structure.
 * @param[in] cfg The configuration.
 * @retval -1 Parameter failure.
 * @retval  0 Success.
 */
static int simulator_open(
		struct device_t * device,
		const struct device_config_t * cfg)
{
	struct sigaction act;
	int pfd[2];
	struct itimerval timerval;

	UNUSED_ARG(cfg);

	if (device == NULL)
		return -1;
	if (device->fd >= 0)
		return 0;

	if (pipe(pfd) < 0) {
		syslog(LOG_CRIT, "unable create pipe");
		return -1;
	}

	/* setup the device and simulator data */
	device->fd = pfd[0];
	device->data = NULL;
	simulator_data.fd = pfd[1];
	simulator_data.index = 0;
	simulator_data.data = DATA;
	simulator_data.len = sizeof(DATA);

	/* setup periodic handler to send simulated GPS data */
	memset(&timerval, 0, sizeof(timerval));
	timerval.it_interval.tv_sec = 0;
	timerval.it_interval.tv_usec = 5000;
	timerval.it_value.tv_sec = 0;
	timerval.it_value.tv_usec = 5000;

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = alarm_handler;
	act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGALRM, &act, NULL) < 0) {
		syslog(LOG_CRIT, "unable to signal action for SIGALRM");
		return -1;
	} else {
		setitimer(ITIMER_REAL, &timerval, NULL);
	}

	return 0;
}

/**
 * Closes the device.
 *
 * @param[inout] device The device descriptor device.
 * @retval -1 Parameter failure.
 * @retval  0 Success.
 */
static int simulator_close(struct device_t * device)
{
	if (device == NULL)
		return -1;
	if (device->fd < 0)
		return 0;

	sigaction(SIGALRM, NULL, NULL);
	close(simulator_data.fd);
	close(device->fd);

	device->fd = -1;
	device->data = NULL;
	return 0;
}

/**
 * Reads data from the simulator.
 *
 * @param[out] device The device descriptor.
 * @param[out] buf The buffer to contain the read data.
 * @param[in] size Buffer size in bytes.
 * @retval -1 Parameter failure.
 * @return Number of read bytes.
 */
static int simulator_read(
		struct device_t * device,
		char * buf,
		uint32_t size)
{
	if (device == NULL)
		return -1;
	if (buf == NULL)
		return -1;
	if (device->fd < 0)
		return -1;
	if (size == 0)
		return 0;

	return read(device->fd, buf, size);
}

/**
 * Does nothing. Returns always -1.
 */
static int simulator_write(
		struct device_t * device,
		const char * buf,
		uint32_t size)
{
	UNUSED_ARG(device);
	UNUSED_ARG(buf);
	UNUSED_ARG(size);
	return -1;
}

/**
 * Structure to describe the simulator device.
 */
const struct device_operations_t simulator_serial_seatalk_operations =
{
	.open = simulator_open,
	.close = simulator_close,
	.read = simulator_read,
	.write = simulator_write,
};

