#include <common/fileutil.h>
#include <sys/stat.h>
#include <string.h>

/**
 * Checks whether or not a file is readable.
 *
 * @param[in] path Path to the file.
 * @retval 1 File exists and is readable.
 * @retval 0 File is not readable.
 */
int file_is_readable(const char * path)
{
	int rc;
	struct stat s;

	if (!path) return 0;
	if (strlen(path) <= 0) return 0;

	rc = stat(path, &s);
	if (rc < 0) return 0;

	if (!S_ISREG(s.st_mode)) return 0;
	if (!(s.st_mode & S_IRUSR)) return 0;

	return 1;
}

/**
 * Checks whether or not a file is writable.
 *
 * @param[in] path Path to the file.
 * @retval 1 File exists and is writable.
 * @retval 0 File is not writable.
 */
int file_is_writable(const char * path)
{
	int rc;
	struct stat s;

	if (!path) return 0;
	if (strlen(path) <= 0) return 0;

	rc = stat(path, &s);
	if (rc < 0) return 0;

	if (!S_ISREG(s.st_mode)) return 0;
	if (!(s.st_mode & S_IWUSR)) return 0;

	return 1;
}

