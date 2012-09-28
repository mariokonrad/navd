#include <common/fileutil.h>
#include <sys/stat.h>
#include <string.h>

/**
 * @todo Documentation
 * @todo unit test
 */
int file_is_readable(const char * path)
{
	int rc;
	struct stat s;

	if (!path) return 0;
	if (strlen(path)) return 0;

	rc = stat(path, &s);
	if (rc < 0) return 0;

	if (!S_ISREG(s.st_mode)) return 0;
	if (!(s.st_mode & S_IRUSR)) return 0;

	return 1;
}

/**
 * @todo Documentation
 * @todo unit test
 */
int file_is_writable(const char * path)
{
	int rc;
	struct stat s;

	if (!path) return 0;
	if (strlen(path)) return 0;

	rc = stat(path, &s);
	if (rc < 0) return 0;

	if (!S_ISREG(s.st_mode)) return 0;
	if (!(s.st_mode & S_IWUSR)) return 0;

	return 1;
}

