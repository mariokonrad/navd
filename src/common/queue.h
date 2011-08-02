#ifndef __COMMON__QUEUE__H__
#define __COMMON__QUEUE__H__

#include <pthread.h>
#include <stdint.h>

/**
 * @todo Documenation
 */
struct pool_t
{
	int (*write)(struct pool_t *, void *, uint32_t);
	int (*read)(struct pool_t *, void *, uint32_t);

	void * data;
	uint32_t size;
};

/**
 * @todo Documenation
 */
struct queue_t
{
	pthread_mutex_t mtx;
	pthread_cond_t not_full;
	pthread_cond_t not_empty;

	struct pool_t * pool;
	uint32_t n;
	uint32_t read;
	uint32_t write;
};

int queue_destroy(struct queue_t * q);
int queue_init(struct queue_t * q, struct pool_t * pool);
int queue_count(struct queue_t * q, uint32_t * count);
int queue_write_noblock(struct queue_t * q, void * data);
int queue_write(struct queue_t * q, void * data);
int queue_read_noblock(struct queue_t * q, void * data);
int queue_read(struct queue_t * q, void * data);

#endif
