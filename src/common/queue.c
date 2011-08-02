#include <common/queue.h>

/**
 * Destroys the specified queue (not data it is currently holding, e.g. it
 * does not touch the pool).
 *
 * @param[inout] q The queue to destroy.
 * @retval  0 success
 * @retval -1 failure
 *
 * @todo Test
 */
int queue_destroy(struct queue_t * q)
{
	int rc;
	int res = 0;

	if (q == NULL) return -1;

	rc = pthread_mutex_destroy(&q->mtx);
	if (rc < 0) res = -1;

	rc = pthread_cond_destroy(&q->not_full);
	if (rc < 0) res = -1;

	rc = pthread_cond_destroy(&q->not_empty);
	if (rc < 0) res = -1;

	return res;
}

/**
 * Initializes the specified queue.
 *
 * @param[out] q The queue to be initialized.
 * @param[in] pool The data pool which holds the concrete data.
 * @retval  0 success
 * @retval -1 failure
 *
 * @todo Test
 */
int queue_init(struct queue_t * q, struct pool_t * pool)
{
	int rc;

	if (q == NULL || pool == NULL) return -1;

	rc = pthread_mutex_init(&q->mtx, NULL);
	if (rc < 0) return -1;

	rc = pthread_cond_init(&q->not_full, NULL);
	if (rc < 0) goto clean_mutex;

	rc = pthread_cond_init(&q->not_empty, NULL);
	if (rc < 0) goto clean_cond;

	q->pool = pool;
	q->n = 0;
	q->read = 0;
	q->write = 0;

	return 0;

clean_cond:
	pthread_cond_destroy(&q->not_full);
clean_mutex:
	pthread_mutex_destroy(&q->mtx);
	return -1;
}

/**
 * Returns the number of elements currently within the queue.
 *
 * @param[in] q The queue
 * @param[out] count Number of elements within queue.
 * @retval  0 success
 * @retval -1 failure
 *
 * @todo Test
 */
int queue_count(struct queue_t * q, uint32_t * count)
{
	if (q == NULL || q->pool == NULL || count == NULL) return -1;
	if (pthread_mutex_lock(&q->mtx) < 0) return -1;
	*count = q->n;
	if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
	return 0;
}

/**
 * @todo Documentation
 * @todo Test
 */
int queue_write_noblock(struct queue_t * q, void * data)
{
	if (q == NULL || q->pool == NULL) return -1;

	if (pthread_mutex_lock(&q->mtx) < 0) return -1;
	if (q->n == q->pool->size) {
		if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
		return -2;
	}
	q->pool->write(q->pool, data, q->write);
	q->write = (q->write + 1) % q->pool->size;
	q->n++;
	if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
	if (pthread_cond_broadcast(&q->not_empty) < 0) return -1;

	return 0;
}

/**
 * @todo Documentation
 * @todo Test
 */
int queue_write(struct queue_t * q, void * data)
{
	if (q == NULL || q->pool == NULL) return -1;

	if (pthread_mutex_lock(&q->mtx) < 0) return -1;
	while (q->n == q->pool->size) {
		if (pthread_cond_wait(&q->not_full, &q->mtx) < 0) return -1;
	}
	q->pool->write(q->pool, data, q->write);
	q->write = (q->write + 1) % q->pool->size;
	q->n++;
	if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
	if (pthread_cond_broadcast(&q->not_empty) < 0) return -1;

	return 0;
}

/**
 * @todo Documentation
 * @todo Test
 */
int queue_read_noblock(struct queue_t * q, void * data)
{
	if (q == NULL || q->pool == NULL) return -1;

	if (pthread_mutex_lock(&q->mtx) < 0) return -1;
	while (q->n == 0) {
		if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
		return -2;
	}
	q->pool->read(q->pool, data, q->read);
	q->read = (q->read + 1) % q->pool->size;
	q->n--;
	if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
	if (pthread_cond_broadcast(&q->not_full) < 0) return -1;

	return 0;
}

/**
 * @todo Documentation
 * @todo Test
 */
int queue_read(struct queue_t * q, void * data)
{
	if (q == NULL || q->pool == NULL) return -1;

	if (pthread_mutex_lock(&q->mtx) < 0) return -1;
	while (q->n == 0) {
		if (pthread_cond_wait(&q->not_empty, &q->mtx) < 0) return -1;
	}
	q->pool->read(q->pool, data, q->read);
	q->read = (q->read + 1) % q->pool->size;
	q->n--;
	if (pthread_mutex_unlock(&q->mtx) < 0) return -1;
	if (pthread_cond_broadcast(&q->not_full) < 0) return -1;

	return 0;
}

