#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

uint32_t crc32_init(void);
uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len);
uint32_t crc32_finalize(uint32_t crc);

#endif
