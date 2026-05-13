#include "boot_flag.h"

// Boot flag placed in SRAM via linker script section .boot_flag
volatile boot_flag_t boot_flag __attribute__((section(".boot_flag"))) = {
    .flag = BOOT_FLAG_APP,
    .cmd = BOOT_CMD_NONE,
    .flash_addr = 0U,
    .flash_size = 0U
};
