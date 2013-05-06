#include <common/endian.h>

/**
 * Structure to perform endian test.
 */
union endian_test_t {
	uint16_t x;
	uint8_t c[sizeof(uint16_t)];
};

static const union endian_test_t _endian_test = { 0x01 };

/**
 * Checks whether or not the machine is little endian.
 *
 * @retval 0 Big endian.
 * @retval 1 Little endian.
 */
int endian_is_little(void)
{
	static int little = -1;
	if (little < 0) little = _endian_test.c[0] == 0x01;
	return little;
}

/**
 * Swaps byte order.
 *
 * @param[in] v Value to swap.
 * @return Swapped value.
 */
uint16_t byte_swap_16(uint16_t v)
{
	return 0
		| ((v & 0xff00) >> 8)
		| ((v & 0x00ff) << 8)
		;
}

/**
 * Swaps byte order.
 *
 * @param[in] v Value to swap.
 * @return Swapped value.
 */
uint32_t byte_swap_32(uint32_t v)
{
	return 0
		| ((v & 0xff000000) >> 24)
		| ((v & 0x00ff0000) >>  8)
		| ((v & 0x0000ff00) <<  8)
		| ((v & 0x000000ff) << 24)
		;
}

/**
 * Swaps byte order.
 *
 * @param[in] v Value to swap.
 * @return Swapped value.
 */
uint64_t byte_swap_64(uint64_t v)
{
	return 0
		| ((v & 0xff00000000000000) >> 56)
		| ((v & 0x00ff000000000000) >> 40)
		| ((v & 0x0000ff0000000000) >> 24)
		| ((v & 0x000000ff00000000) >>  8)
		| ((v & 0x00000000ff000000) <<  8)
		| ((v & 0x0000000000ff0000) << 24)
		| ((v & 0x000000000000ff00) << 40)
		| ((v & 0x00000000000000ff) << 56)
		;
}

/**
 * Convertes the value from host to network byte order.
 *
 * @param[in] v Value to convert.
 * @return Converted value.
 */
uint16_t endian_hton_16(uint16_t v)
{
	return endian_is_little() ? byte_swap_16(v) : v;
}

/**
 * Convertes the value from host to network byte order.
 *
 * @param[in] v Value to convert.
 * @return Converted value.
 */
uint32_t endian_hton_32(uint32_t v)
{
	return endian_is_little() ? byte_swap_32(v) : v;
}

/**
 * Convertes the value from host to network byte order.
 *
 * @param[in] v Value to convert.
 * @return Converted value.
 */
uint64_t endian_hton_64(uint64_t v)
{
	return endian_is_little() ? byte_swap_64(v) : v;
}

/**
 * Convertes the value from network to host byte order.
 *
 * @param[in] v Value to convert.
 * @return Converted value.
 */
uint16_t endian_ntoh_16(uint16_t v)
{
	return endian_is_little() ? byte_swap_16(v) : v;
}

/**
 * Convertes the value from network to host byte order.
 *
 * @param[in] v Value to convert.
 * @return Converted value.
 */
uint32_t endian_ntoh_32(uint32_t v)
{
	return endian_is_little() ? byte_swap_32(v) : v;
}

/**
 * Convertes the value from network to host byte order.
 *
 * @param[in] v Value to convert.
 * @return Converted value.
 */
uint64_t endian_ntoh_64(uint64_t v)
{
	return endian_is_little() ? byte_swap_64(v) : v;
}

