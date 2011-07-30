#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <common/macros.h>
#include <nmea/nmea_sentence_gprmc.h>
#include <message.h>
#include <unistd.h>

struct strategy_t
{
	const char * name;
	int (*func)(void *);
};

struct action_t
{
	const struct strategy_t * strategy;
	void * data;
};

static void * thread_func(void * ptr)
{
	int rc;
	struct action_t * action = NULL;

	if (ptr != NULL) {
		action = (struct action_t *)ptr;
		if (action->strategy == NULL || action->strategy->func == NULL) {
			fprintf(stderr, "%s:%d: error executable strategy\n", __FILE__, __LINE__);
		} else {
			rc = action->strategy->func(action->data);
			if (rc < 0) {
				fprintf(stderr, "%s:%d: error in strategy '%s', rc=%d\n", __FILE__, __LINE__, action->strategy->name, rc);
			}
		}
	} else {
		fprintf(stderr, "%s:%d: no action specified\n", __FILE__, __LINE__);
	}
	pthread_exit(NULL);
}


struct gps_simulation_data_t
{
	int number_of_messages;
	/* TODO: message destination */
};

/* Sends periodic position updates, RMC sentences, once per second */
static int execute_gps_simulation(void * ptr)
{
	int rc;
	struct message_t message;
	struct nmea_rmc_t * rmc;
	char buf[NMEA_MAX_SENTENCE];
	struct gps_simulation_data_t * data;
	int message_no;

	if (ptr == NULL) {
		fprintf(stderr, "%s:%d: invalid configuration data\n", __FILE__, __LINE__);
		return -1;
	}

	data = (struct gps_simulation_data_t *)ptr;

	message.type = MSG_NMEA;
	message.data.nmea.type = NMEA_RMC;
	rmc = &message.data.nmea.sentence.rmc;
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

	rc = nmea_write(buf, sizeof(buf), &message.data.nmea);
	if (rc < 0) {
		fprintf(stderr, "%s:%d: invalid RMC data, rc=%d\n", __FILE__, __LINE__, rc);
		return -1;
	}

	for (message_no = 0; (data->number_of_messages < 0) || (message_no < data->number_of_messages); ++message_no) {
		printf("%s:%d: sending [%s]\n", __FILE__, __LINE__, buf);
		/* TODO */
		sleep(1);
	}

	return 0;
}

static const struct strategy_t strategy_gps_simulation =
{
	.name = "gps_simulation",
	.func = execute_gps_simulation,
};


int main(int argc, char ** argv)
{
	int rc;
	pthread_t tid;
	struct action_t action;

	struct gps_simulation_data_t simulation_data;

	UNUSED_ARG(argc);
	UNUSED_ARG(argv);

	action.strategy = &strategy_gps_simulation;
	action.data = &simulation_data;
	simulation_data.number_of_messages = 5;

	rc = pthread_create(&tid, NULL, thread_func, (void *)&action);
	if (rc < 0) {
		perror("pthread_create");
		exit(-1);
	}

	rc = pthread_join(tid, NULL);
	if (rc < 0) {
		perror("pthread_join");
		exit(-1);
	}

	return EXIT_SUCCESS;
}

