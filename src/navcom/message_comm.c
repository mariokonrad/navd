#include <navcom/message_comm.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/**
 * Reads the message from the specified file descriptor.
 *
 * @param[in] fd File descriptor to read from.
 * @param[out] msg The message which is received.
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
int message_read(int fd, struct message_t * msg)
{
	int rc;

	if (fd < 0)
		return EXIT_FAILURE;
	if (!msg)
		return EXIT_FAILURE;
	rc = read(fd, msg, sizeof(struct message_t));
	if (rc < 0) {
		syslog(LOG_ERR, "unable to read message: %s", strerror(errno));
		return EXIT_FAILURE;
	}
	if (rc != (int)sizeof(struct message_t) || rc == 0) {
		syslog(LOG_ERR, "cannot read message, rc=%d", rc);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * Writes the message to the file descriptor.
 *
 * @param[in] fd File descriptor to write to.
 * @param[in] msg Message to write.
 * @retval EXIT_SUCCESS
 * @retval EXIT_FAILURE
 */
int message_write(int fd, const struct message_t * msg)
{
	int rc;

	if (fd < 0)
		return EXIT_FAILURE;
	if (!msg)
		return EXIT_FAILURE;
	rc = write(fd, msg, sizeof(struct message_t));
	if (rc < 0) {
		syslog(LOG_DEBUG, "unable to write message: %s", strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

