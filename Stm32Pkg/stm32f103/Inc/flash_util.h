#ifndef FLASH_UTIL_H
#define FLASH_UTIL_H

#include <stdint.h>

/**
 * @brief Unlock FLASH for programming
 */
void flash_unlock(void);

/**
 * @brief Lock FLASH after programming
 */
void flash_lock(void);

/**
 * @brief Erase a page in FLASH
 * @param address Start address of the page
 */
void flash_erase_page(uint32_t address);

/**
 * @brief Write a word to FLASH
 * @param address Destination address (must be word aligned)
 * @param data Word to write
 */
void flash_write_word(uint32_t address, uint32_t data);

#endif // FLASH_UTIL_H
