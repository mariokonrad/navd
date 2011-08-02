#include <common/endian.h>

/**
 * @todo Documenation
 */
union endian_test_t {
	uint16_t x;
	uint8_t c[sizeof(uint16_t)];
};

static const union endian_test_t _endian_test = { 0x01 };

/**
 * @todo Documenation
 */
static int endian_is_little(void)
{
	static int little = -1;
	if (little < 0) little = _endian_test.c[0] == 0x01;
	return little;
}

/**
 * @todo Documenation
 */
uint16_t byte_swap_16(uint16_t v)
{
	return 0
		| ((v & 0xff00) >> 8)
		| ((v & 0x00ff) << 8)
		;
}

/**
 * @todo Documenation
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
 * @todo Documenation
 */
uint16_t endian_hton_16(uint16_t v)
{
	return endian_is_little() ? byte_swap_16(v) : v;
}

/**
 * @todo Documenation
 */
uint32_t endian_hton_32(uint32_t v)
{
	return endian_is_little() ? byte_swap_32(v) : v;
}

/**
 * @todo Documenation
 */
uint16_t endian_ntoh_16(uint16_t v)
{
	return endian_is_little() ? byte_swap_16(v) : v;
}

/**
 * @todo Documenation
 */
uint32_t endian_ntoh_32(uint32_t v)
{
	return endian_is_little() ? byte_swap_32(v) : v;
}

