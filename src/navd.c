#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <common/macros.h>
#include <common/queue.h>
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
	void * config;
};

/* {{{ thread */

static void * thread_func(void * ptr)
{
	int rc;
	struct action_t * action = NULL;

	if (ptr != NULL) {
		action = (struct action_t *)ptr;
		if (action->strategy == NULL || action->strategy->func == NULL) {
			fprintf(stderr, "%s:%d: error executable strategy\n", __FILE__, __LINE__);
		} else {
			rc = action->strategy->func(action->config);
			if (rc < 0) {
				fprintf(stderr, "%s:%d: error in strategy '%s', rc=%d\n", __FILE__, __LINE__, action->strategy->name, rc);
			}
		}
	} else {
		fprintf(stderr, "%s:%d: no action specified\n", __FILE__, __LINE__);
	}
	pthread_exit(NULL);
}

/* thread }}} */

/* {{{ gps simulation */

struct gps_simulation_config_t
{
	struct queue_t * message_queue;
	int number_of_messages;
};

/* Sends periodic position updates, RMC sentences, once per second */
static int execute_gps_simulation(void * ptr)
{
	int rc;
	struct message_t message;
	struct nmea_rmc_t * rmc;
	char buf[NMEA_MAX_SENTENCE];
	struct gps_simulation_config_t * config;
	int message_no;

	if (ptr == NULL) {
		fprintf(stderr, "%s:%d: invalid configuration\n", __FILE__, __LINE__);
		return -1;
	}

	config = (struct gps_simulation_config_t *)ptr;

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

	/* queue_read_timeout */

	for (message_no = 0; (config->number_of_messages < 0) || (message_no < config->number_of_messages); ++message_no) {
		printf("%s:%d: sending [%s]\n", __FILE__, __LINE__, buf);
		if (queue_write(config->message_queue, &message) < 0) {
			fprintf(stderr, "%s:%d: error while sending messages\n", __FILE__, __LINE__);
			return -1;
		}
		sleep(1);
	}

	return 0;
}

static const struct strategy_t strategy_gps_simulation =
{
	.name = "gps_simulation",
	.func = execute_gps_simulation,
};

/* gps simulation }}} */

/* {{{ hub */

struct hub_config_t
{
	struct queue_t * message_queue;
};

static int execute_hub(void * ptr)
{
	struct hub_config_t * config;
	struct message_t message;
	char buf[NMEA_MAX_SENTENCE];
	int rc;
	int terminate = 0;

	if (ptr == NULL) {
		fprintf(stderr, "%s:%d: invalid configuration\n", __FILE__, __LINE__);
		return -1;
	}

	config = (struct hub_config_t *)ptr;

	for (; !terminate;) {
		if (queue_read(config->message_queue, &message) < 0) {
			fprintf(stderr, "%s:%d: error while receiving messages\n", __FILE__, __LINE__);
			return -1;
		}
		switch (message.type) {
			case MSG_SYSTEM:
				terminate = message.data.system == SYSTEM_TERMINATE;
				break;

			case MSG_NMEA:
				memset(buf, 0, sizeof(buf));
				rc = nmea_write(buf, sizeof(buf), &message.data.nmea);
				if (rc < 0) {
					fprintf(stderr, "%s:%d: error while writing NMEA data to buffer\n", __FILE__, __LINE__);
					continue;
				}
				printf("%s:%d: received message: [%s]\n", __FILE__, __LINE__, buf);
				break;

			default:
				fprintf(stderr, "%s:%d: unknown message type: %u, ignoring message\n", __FILE__, __LINE__, message.type);
				break;
		}
	}

	return 0;
}

static const struct strategy_t strategy_hub =
{
	.name = "hub",
	.func = execute_hub,
};

/* hub }}}*/

static int message_pool_write(struct pool_t * pool, void * data, uint32_t index)
{
	if (pool == NULL || data == NULL) return -1;
	((struct message_t *)pool->data)[index] = *(struct message_t *)data;
	return 0;
}

static int message_pool_read(struct pool_t * pool, void * data, uint32_t index)
{
	if (pool == NULL || data == NULL) return -1;
	*(struct message_t *)data = ((struct message_t *)pool->data)[index];
	return 0;
}

int main(int argc, char ** argv)
{
	int rc;
	struct message_t msg_terminate;

	struct message_t hub_messages[8];
	struct pool_t hub_message_pool;
	struct queue_t hub_message_queue;

	pthread_t tid_hub;
	struct action_t action_hub;
	struct hub_config_t hub_config;

	struct message_t gps_sim_messages[8];
	struct pool_t gps_sim_message_pool;
	struct queue_t gps_sim_message_queue;

	pthread_t tid_gps_simulation;
	struct action_t action_gps_simulation;
	struct gps_simulation_config_t simulation_config;

	UNUSED_ARG(argc);
	UNUSED_ARG(argv);

	/* set up system messages */

	msg_terminate.type = MSG_SYSTEM;
	msg_terminate.data.system = SYSTEM_TERMINATE;

	/* set up message queues */

	memset(hub_messages, 0, sizeof(hub_messages));
	hub_message_pool.data = hub_messages;
	hub_message_pool.size = sizeof(hub_messages) / sizeof(hub_messages[0]);
	hub_message_pool.write = &message_pool_write;
	hub_message_pool.read = &message_pool_read;
	rc = queue_init(&hub_message_queue, &hub_message_pool);
	if (rc < 0) {
		fprintf(stderr, "error: cannot initialize hub_message_queue\n");
		exit(-1);
	}

	memset(gps_sim_messages, 0, sizeof(gps_sim_messages));
	gps_sim_message_pool.data = gps_sim_messages;
	gps_sim_message_pool.size = sizeof(gps_sim_messages) / sizeof(gps_sim_messages[0]);
	gps_sim_message_pool.write = &message_pool_write;
	gps_sim_message_pool.read = &message_pool_read;
	rc = queue_init(&gps_sim_message_queue, &gps_sim_message_pool);
	if (rc < 0) {
		fprintf(stderr, "error: cannot initialize gps_sim_message_queue\n");
		exit(-1);
	}

	/* set up activities */

	action_gps_simulation.strategy = &strategy_gps_simulation;
	action_gps_simulation.config = &simulation_config;
	simulation_config.message_queue = &hub_message_queue;
	simulation_config.number_of_messages = 5;

	action_hub.strategy = &strategy_hub;
	action_hub.config = &hub_config;
	hub_config.message_queue = &hub_message_queue;

	/* start thread */

	rc = pthread_create(&tid_hub, NULL, thread_func, (void *)&action_hub);
	if (rc < 0) {
		perror("pthread_create");
		exit(-1);
	}

	rc = pthread_create(&tid_gps_simulation, NULL, thread_func, (void *)&action_gps_simulation);
	if (rc < 0) {
		perror("pthread_create");
		exit(-1);
	}

	/* terminate system after 10s */

	sleep(10);
	queue_write(&hub_message_queue, &msg_terminate);

	/* wait for termination and clean up */

	rc = pthread_join(tid_gps_simulation, NULL);
	if (rc < 0) {
		perror("pthread_join");
		exit(-1);
	}

	rc = pthread_join(tid_hub, NULL);
	if (rc < 0) {
		perror("pthread_join");
		exit(-1);
	}

	rc = queue_destroy(&gps_sim_message_queue);
	if (rc < 0) {
		fprintf(stderr, "error while destroying gps_sim_message_queue\n");
		exit(-1);
	}

	rc = queue_destroy(&hub_message_queue);
	if (rc < 0) {
		fprintf(stderr, "error while destroying hub_message_queue\n");
		exit(-1);
	}

	return EXIT_SUCCESS;
}

