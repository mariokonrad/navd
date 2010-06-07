#include "nmea_util.h"

/*
 * converts a fix point number to float.
 * @param[in] fix the fix point number to convert
 * @param[out] v converted number
 * @retval  0 success
 * @retval -1 failure
 */
int nmea_fix_to_float(const struct nmea_fix_t * fix, float * v)
{
	if (!fix || !v) return -1;
	*v = (float)fix->i + ((float)fix->d / (float)NMEA_FIX_DECIMALS);
	return 0;
}

/*
 * converts a fix point number to double.
 * @param[in] fix the fix point number to convert
 * @param[out] v converted number
 * @retval  0 success
 * @retval -1 failure
 */
int nmea_fix_to_double(const struct nmea_fix_t * fix, double * v)
{
	if (!fix || !v) return -1;
	*v = (double)fix->i + ((double)fix->d / (double)NMEA_FIX_DECIMALS);
	return 0;
}

