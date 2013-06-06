#include <daemon.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

void daemonize(void)
{
	int rc;

	rc = fork();
	if (rc < 0) {
		syslog(LOG_CRIT, "unable to fork");
		exit(EXIT_FAILURE);
	}
	if (rc > 0) {
		exit(rc);
	}
}

