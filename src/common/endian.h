#ifndef __ENDIAN__H__
#define __ENDIAN__H__

#include <stdint.h>

uint16_t byte_swap_16(uint16_t);
uint32_t byte_swap_32(uint32_t);
uint64_t byte_swap_64(uint64_t);

uint16_t endian_hton_16(uint16_t);
uint32_t endian_hton_32(uint32_t);
uint64_t endian_hton_64(uint64_t);

uint16_t endian_ntoh_16(uint16_t);
uint32_t endian_ntoh_32(uint32_t);
uint64_t endian_ntoh_64(uint64_t);

#endif
