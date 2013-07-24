#include <seatalk/seatalk_base.h>
#include <string.h>

/**
 * Initializes the specified structure.
 *
 * @param[out] seatalk The structgure to initialize.
 * @retval  0 Success
 * @retval -1 Parameter error
 */
int seatalk_init(struct seatalk_t * seatalk)
{
	if (seatalk == NULL)
		return -1;
	memset(seatalk, 0, sizeof(struct seatalk_t));
	seatalk->type = 0xff;
	return 0;
}

