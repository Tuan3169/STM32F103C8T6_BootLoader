/*
 * Boot flag definitions - stored in SRAM
 * Used for bootloader to decide: jump to Application or OTA
 */

#ifndef BOOT_FLAG_H
#define BOOT_FLAG_H

#include <stdint.h>

// Boot flag values
#define BOOT_FLAG_APP         0x00000000U  // Jump to Application (default)
#define BOOT_FLAG_OTA         0xDEADBEEFU  // Jump to OTA
#define BOOT_FLAG_BOOTLOADER  0xB00710ADU  // Stay in bootloader
#define BOOT_FLAG_ADDR        0x2004FF80U  // Last 16 bytes of SRAM (0x20000000 + 20K - 128)

// Boot command values
#define BOOT_CMD_NONE         0x00000000U
#define BOOT_CMD_FLASH        0x00000001U

// Boot flag structure (shared in SRAM)
typedef struct {
    uint32_t flag;       // Boot flag value
    uint32_t cmd;        // Boot command
    uint32_t flash_addr; // Flash start address
    uint32_t flash_size; // Flash size
} boot_flag_t;

// Declare boot flag variable (placed in SRAM via linker script)
extern volatile boot_flag_t boot_flag;

// Inline functions to set/get flag
static inline void boot_flag_set_app(void) {
    boot_flag.flag = BOOT_FLAG_APP;
    boot_flag.cmd = BOOT_CMD_NONE;
    boot_flag.flash_addr = 0U;
    boot_flag.flash_size = 0U;
}

static inline void boot_flag_set_ota(void) {
    boot_flag.flag = BOOT_FLAG_OTA;
    boot_flag.cmd = BOOT_CMD_NONE;
    boot_flag.flash_addr = 0U;
    boot_flag.flash_size = 0U;
}

static inline void boot_flag_request_flash(uint32_t addr, uint32_t size) {
    boot_flag.flag = BOOT_FLAG_BOOTLOADER;
    boot_flag.cmd = BOOT_CMD_FLASH;
    boot_flag.flash_addr = addr;
    boot_flag.flash_size = size;
}

static inline void boot_flag_clear(void) {
    boot_flag_set_app();
}

static inline uint32_t boot_flag_get(void) {
    return boot_flag.flag;
}

static inline uint32_t boot_flag_get_cmd(void) {
    return boot_flag.cmd;
}

static inline uint32_t boot_flag_get_flash_addr(void) {
    return boot_flag.flash_addr;
}

static inline uint32_t boot_flag_get_flash_size(void) {
    return boot_flag.flash_size;
}

#endif // BOOT_FLAG_H
