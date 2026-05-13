#include "flash_util.h"

// STM32F103 FLASH registers
#define FLASH_BASE       0x40022000
#define FLASH_ACR        (*(volatile uint32_t*)(FLASH_BASE + 0x00))
#define FLASH_KEYR       (*(volatile uint32_t*)(FLASH_BASE + 0x04))
#define FLASH_OPTKEYR    (*(volatile uint32_t*)(FLASH_BASE + 0x08))
#define FLASH_SR         (*(volatile uint32_t*)(FLASH_BASE + 0x0C))
#define FLASH_CR         (*(volatile uint32_t*)(FLASH_BASE + 0x10))
#define FLASH_AR         (*(volatile uint32_t*)(FLASH_BASE + 0x14))
#define FLASH_OBR        (*(volatile uint32_t*)(FLASH_BASE + 0x1C))
#define FLASH_WRPR       (*(volatile uint32_t*)(FLASH_BASE + 0x20))

// FLASH Key values
#define FLASH_KEY1       0x45670123
#define FLASH_KEY2       0xCDEF89AB

// FLASH Control Register flags
#define FLASH_CR_PG      (1 << 0)  // Programming
#define FLASH_CR_PER     (1 << 1)  // Page Erase
#define FLASH_CR_MER     (1 << 2)  // Mass Erase
#define FLASH_CR_OPTPG   (1 << 4)  // Option Byte Programming
#define FLASH_CR_OPTER   (1 << 5)  // Option Byte Erase
#define FLASH_CR_STRT    (1 << 6)  // Start
#define FLASH_CR_LOCK    (1 << 7)  // Lock
#define FLASH_CR_OPTWRE  (1 << 9)  // Option Write Enable

// FLASH Status Register flags
#define FLASH_SR_BSY     (1 << 0)  // Busy
#define FLASH_SR_PGERR   (1 << 2)  // Programming Error
#define FLASH_SR_WRPRTERR (1 << 4) // Write Protection Error
#define FLASH_SR_EOP     (1 << 5)  // End of Operation

void flash_unlock(void)
{
    if ((FLASH_CR & FLASH_CR_LOCK) == 0) {
        return;  // Already unlocked
    }
    
    FLASH_KEYR = FLASH_KEY1;
    FLASH_KEYR = FLASH_KEY2;
}

void flash_lock(void)
{
    FLASH_CR |= FLASH_CR_LOCK;
}

static void flash_wait_ready(void)
{
    while (FLASH_SR & FLASH_SR_BSY) {
        // Wait for operation to complete
        __asm volatile ("nop");
    }
}

static void flash_clear_status(void)
{
    FLASH_SR |= (FLASH_SR_PGERR | FLASH_SR_WRPRTERR | FLASH_SR_EOP);
}

void flash_erase_page(uint32_t address)
{
    flash_wait_ready();
    flash_clear_status();
    
    // Select page erase
    FLASH_CR |= FLASH_CR_PER;
    
    // Set address
    FLASH_AR = address;
    
    // Start erase
    FLASH_CR |= FLASH_CR_STRT;
    
    // Wait for completion
    flash_wait_ready();
    flash_clear_status();
    
    // Clear page erase flag
    FLASH_CR &= ~FLASH_CR_PER;
}

void flash_write_word(uint32_t address, uint32_t data)
{
    flash_wait_ready();
    flash_clear_status();
    
    // Enable programming
    FLASH_CR |= FLASH_CR_PG;
    
    // Write data as two halfwords (STM32F1 supports halfword programming)
    *(volatile uint16_t*)address = (uint16_t)(data & 0xFFFFU);
    flash_wait_ready();
    flash_clear_status();

    *(volatile uint16_t*)(address + 2U) = (uint16_t)((data >> 16U) & 0xFFFFU);
    flash_wait_ready();
    flash_clear_status();
    
    // Disable programming
    FLASH_CR &= ~FLASH_CR_PG;
}
