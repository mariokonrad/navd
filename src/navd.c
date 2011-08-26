#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <common/macros.h>
#include <message.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <device/serial.h>

static volatile int request_terminate = 0;
static sigset_t signal_mask;

typedef struct config_t {
	int pid;
	int pfd;
	int hub_fd;
} config_t;

static void handle_signal(int sig) /* {{{ */
{
	printf("%s:%d: pid:%d sig:%d\n", __FILE__, __LINE__, getpid(), sig);
	request_terminate = 1;
} /* }}} */

static void daemonize(void) /* {{{ */
{
	int rc;

	rc = fork();
	if (rc < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (rc > 0) {
		exit(rc);
	}
} /* }}} */

static int start_proc(config_t * proc, void (*func)(const config_t *)) /* {{{ */
{
	int pfd[2];
	int rc;

	if (proc == NULL) return -1;
	if (func == NULL) return -1;

	rc = pipe(pfd);
	if (rc < 0) {
		perror("pipe");
		return -1;
	}

	rc = fork();
	if (rc < 0) {
		perror("fork");
		return -1;
	}

	if (rc == 0) {
		proc->pid = getpid();
		proc->pfd = pfd[0];
		close(pfd[1]);  /* close writing end of pipe, child doesn't need it */
		func(proc); /* child code, will not return */
		exit(EXIT_SUCCESS);
	}

	close(pfd[0]); /* close reading end of pipe, parent doesn't need it */

	proc->pid = rc;
	proc->pfd = pfd[1];

	return rc;
} /* }}} */


static void gps_device_serial_demo(const config_t * config) /* <producer> {{{ */
{
	int rc;

	fd_set rfds;
	int fd_max;
	struct message_t msg;
	struct timespec tm;

	struct device_t device;
	const struct device_operations_t * ops = NULL;

	struct nmea_t nmea;
	char buf[NMEA_MAX_SENTENCE + 1];
	int buf_index;
	char c;

	struct serial_config_t serial_config = {
		"/dev/ttyUSB0"
	};

	ops = &serial_device_operations;
	device_init(&device);
	nmea_init(&nmea);
	memset(buf, 0, sizeof(buf));
	buf_index = 0;

	rc = ops->open(&device, (const struct device_config_t *)&serial_config);
	if (rc < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	while (!request_terminate) {
		fd_max = -1;
		FD_ZERO(&rfds);
		FD_SET(config->pfd, &rfds);
		if (config->pfd > fd_max) fd_max = config->pfd;
		FD_SET(device.fd, &rfds);
		if (device.fd > fd_max) fd_max = device.fd;

		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(fd_max + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (FD_ISSET(device.fd, &rfds)) {
			rc = ops->read(&device, &c, sizeof(c));
			if (rc < 0) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (rc != sizeof(c)) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			switch (c) {
				case '\r':
					break;
				case '\n':
					rc = nmea_read(&nmea, buf);
					if (rc == 0) {
						printf("OK : [%s]\n", buf);
						/* TODO: send message */
					} else if (rc == 1) {
						printf("[%s] : UNKNOWN SENTENCE\n", buf);
					} else if (rc == -2) {
						printf("[%s] : CHECKSUM ERROR\n", buf);
					} else {
						fprintf(stderr, "parameter error\n");
						exit(EXIT_FAILURE);
					}
					buf_index = 0;
					buf[buf_index] = 0;
					break;
				default:
					if (buf_index < NMEA_MAX_SENTENCE) {
						buf[buf_index++] = c;
					} else {
						fprintf(stderr, "sentence too long, discarding\n");
						buf_index = 0;
					}
					buf[buf_index] = 0;
					break;
			}
		}

		if (FD_ISSET(config->pfd, &rfds)) {
			rc = read(config->pfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "error: cannot read message\n");
				continue;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							rc = ops->close(&device);
							if (rc < 0) {
								perror("close");
								exit(EXIT_FAILURE);
							}
							exit(EXIT_SUCCESS);
						default:
							break;
					}
				default:
					break;
			}
			continue;
		}
	}
} /* }}} */

static void gps_simulator(const config_t * config) /* <producer> {{{ */
{
	fd_set rfds;
	struct message_t msg;
	struct message_t sim_message;
	int rc;
	struct timespec tm;

	struct nmea_rmc_t * rmc;
	char buf[NMEA_MAX_SENTENCE];

	sim_message.type = MSG_NMEA;
	sim_message.data.nmea.type = NMEA_RMC;
	rmc = &sim_message.data.nmea.sentence.rmc;
	rmc->time.h = 20;
	rmc->time.m = 15;
	rmc->time.s = 30;
	rmc->time.ms = 0;
	rmc->status = NMEA_STATUS_OK;
	rmc->lat.d = 47;
	rmc->lat.m = 8;
	rmc->lat.s.i = 0;
	rmc->lat.s.d = 0;
	rmc->lat_dir = NMEA_NORTH;
	rmc->lon.d = 3;
	rmc->lon.m = 20;
	rmc->lon.s.i = 0;
	rmc->lon.s.d = 0;
	rmc->lon_dir = NMEA_EAST;
	rmc->sog.i = 0;
	rmc->sog.d = 0;
	rmc->head.i = 0;
	rmc->head.d = 0;
	rmc->date.y = 0;
	rmc->date.m = 1;
	rmc->date.d = 1;
	rmc->m.i = 0;
	rmc->m.d = 0;
	rmc->m_dir = NMEA_WEST;
	rmc->sig_integrity = NMEA_SIG_INT_SIMULATED;

	rc = nmea_write(buf, sizeof(buf), &sim_message.data.nmea);
	if (rc < 0) {
		fprintf(stderr, "%s:%d: invalid RMC data, rc=%d\n", __FILE__, __LINE__, rc);
		exit(EXIT_FAILURE);
	}

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->pfd, &rfds);

		tm.tv_sec = 1;
		tm.tv_nsec = 0;

		rc = pselect(config->pfd + 1, &rfds, NULL, NULL, &tm, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (rc < 0 && errno == EINTR) {
			break;
		}

		if (rc == 0) { /* timeout */
			rc = write(config->hub_fd, &sim_message, sizeof(sim_message));
			if (rc < 0) {
				perror("write");
			}
			continue;
		}

		if (FD_ISSET(config->pfd, &rfds)) {
			rc = read(config->pfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "error: cannot read message\n");
				continue;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							exit(EXIT_SUCCESS);
						default:
							break;
					}
				default:
					break;
			}
			continue;
		}
	}
} /* }}} */

static void message_logger(const config_t * config) /* <consumer> {{{ */
{
	fd_set rfds;
	struct message_t msg;
	int rc;
	char buf[NMEA_MAX_SENTENCE];

	while (!request_terminate) {
		FD_ZERO(&rfds);
		FD_SET(config->pfd, &rfds);

		rc = pselect(config->pfd + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (rc < 0 && errno == EINTR) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(config->pfd, &rfds)) {
			rc = read(config->pfd, &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "error: cannot read message\n");
				continue;
			}
			switch (msg.type) {
				case MSG_SYSTEM:
					switch (msg.data.system) {
						case SYSTEM_TERMINATE:
							exit(EXIT_SUCCESS);
						default:
							break;
					}
					break;

				case MSG_NMEA:
					memset(buf, 0, sizeof(buf));
					rc = nmea_write(buf, sizeof(buf), &msg.data.nmea);
					if (rc < 0) {
						fprintf(stderr, "%s:%d: error while writing NMEA data to buffer\n", __FILE__, __LINE__);
						continue;
					}
					printf("%s:%d: received message: [%s]\n", __FILE__, __LINE__, buf);
					break;

				default:
					printf("%s:%d: log  : %08x\n", __FILE__, __LINE__, msg.type);
					break;
			}
			continue;
		}
	}
} /* }}} */


int main(int argc, char ** argv)
{
	config_t cfg_sim;
	config_t cfg_log;

	struct message_t msg;
	int pfd[2];
	int rc;
	fd_set rfds;
	int num_msg;

	sigset_t mask;
	struct sigaction act;

	/* daemonize process */

	if (argc == 2 && strcmp(argv[1], "-d") == 0) {
		daemonize();
	}

	/* set up signal handling (active also for all subprocesses, if masks are global) */

	memset(&act, 0, sizeof(act));
	act.sa_handler = handle_signal;
	if (sigaction(SIGTERM, &act, NULL) < 0) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	if (sigprocmask(SIG_BLOCK, &mask, &signal_mask) < 0) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}

	/* set up pipe from subprocesses to hub */

	rc = pipe(pfd);
	if (rc < 0) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	/* start subprocesses */

	cfg_log.hub_fd = pfd[1];
	if (start_proc(&cfg_log, message_logger) < 0) {
		return EXIT_FAILURE;
	}

	cfg_sim.hub_fd = pfd[1];
	if (start_proc(&cfg_sim, gps_simulator) < 0) {
		return EXIT_FAILURE;
	}

	/* main / hub process */

	num_msg = 20;
	for (; !request_terminate;) {
		FD_ZERO(&rfds);
		FD_SET(pfd[0], &rfds);

		rc = pselect(pfd[0] + 1, &rfds, NULL, NULL, NULL, &signal_mask);
		if (rc < 0 && errno != EINTR) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (request_terminate) {
			break;
		} else if (rc == 0) {
			continue;
		}

		if (FD_ISSET(pfd[0], &rfds)) {
			rc = read(pfd[0], &msg, sizeof(msg));
			if (rc < 0) {
				perror("read");
				continue;
			}
			if (rc != (int)sizeof(msg)) {
				fprintf(stderr, "%s:%d: error: cannot read message\n", __FILE__, __LINE__);
				continue;
			}

			/* TODO: routing table */

			switch (msg.type) {
				default:
					printf("%s:%d: route: %08x\n", __FILE__, __LINE__, msg.type);
					rc = write(cfg_log.pfd, &msg, sizeof(msg));
					if (rc < 0) {
						perror("write");
					}
					break;
			}

			/* TEMP: terminate after num_msg */
			--num_msg;
			if (num_msg <= 0) {
				break;
			}
		}
	}

	/* request subprocesses to terminate, gracefully, and wait for their termination */

	msg.type = MSG_SYSTEM;
	msg.data.system = SYSTEM_TERMINATE;
	rc = write(cfg_sim.pfd, &msg, sizeof(msg));
	if (rc < 0) {
		perror("write");
	}
	rc = write(cfg_log.pfd, &msg, sizeof(msg));
	if (rc < 0) {
		perror("write");
	}

	waitpid(cfg_sim.pid, NULL, 0);
	waitpid(cfg_log.pid, NULL, 0);

	return EXIT_SUCCESS;
}

