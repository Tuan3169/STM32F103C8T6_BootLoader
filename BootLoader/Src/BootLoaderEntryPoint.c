/*****************************************************************************************************************
 * @Filename: BootLoaderEntryPoint.c  * @Author: Đinh Văn Tuấn
 * @Date 2026-05-01
 * @Version: 1.0
 * @Description: Bootloader entry point.
 * 
 * Copyright (c) 2026 []. All rights reserved.
 * Licensed under the MIT License.
 ****************************************************************************************************************/


//################################################# INCLUDE HEARDER #############################################
#include <stdint.h>
#include <stdbool.h>
#include <ProjectConfig.h>
#include "../../Stm32Pkg/stm32f103/Inc/uart_regs.h"
#include "../../Stm32Pkg/stm32f103/Inc/SystemClock.h"
#include "../../Stm32Pkg/stm32f103/Inc/Frequency.h"
#include "../../Stm32Pkg/stm32f103/Inc/Timer.h"
#include "sha256.h"
#include "crc32.h"
#include "../../boot_flag.h"
#include "../../Stm32Pkg/stm32f103/Inc/flash_util.h"


//#################################################     DEFINE    ###############################################
///define APP_START_ADDRESS 0x08001000
// Define SCB for Vector Table Relocation
#define SCB_BASE        0xE000ED00
#define SCB_VTOR        (*(volatile uint32_t*)(SCB_BASE + 0x08))
#define SCB_AIRCR       (*(volatile uint32_t*)(SCB_BASE + 0x0C))
#define SCB_AIRCR_VECTKEY (0x5FAU << 16)
#define SCB_AIRCR_SYSRESETREQ (1U << 2)

// register address
#define RCC_BASE 0x40021000
#define GPIOA_BASE 0x40010800
#define GPIOC_BASE 0x40011000

#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOA_CRL *(volatile uint32_t *)(GPIOA_BASE + 0x00)
#define GPIOA_IDR *(volatile uint32_t *)(GPIOA_BASE + 0x08)
#define GPIOA_ODR *(volatile uint32_t *)(GPIOA_BASE + 0x0C)
#define GPIOC_CRH *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR *(volatile uint32_t *)(GPIOC_BASE + 0x0C)

// bit fields
#define RCC_IOPAEN (1 << 2)
#define RCC_IOPCEN (1 << 4)
#define GPIOA0 (1UL << 0)
#define GPIOA1 (1UL << 1)
#define GPIOC13 (1UL << 13)



static const uint8_t k_uart_greeting[] = "STM32 Hello Man!!\n";
static const char k_uart_ready[] = "READY\n";
static const char k_uart_ack[] = "ACK\n";
static const char k_uart_done[] = "DONE\n";
static const char k_uart_err[] = "ERR\n";

char debug_buf[32];

//#################################################     TYPEDEF   ###############################################


//#################################################     VARIABLE  ###############################################
#define UART_RX_RING_SIZE 4096U
#define UART_CMD_MAX_LEN 128U
#define UART_BLOCK_SIZE 1024U
#define FLASH_MAX_RANGE (64U * 1024U)
#define FLASH_PAGE_SIZE 1024U

#define MESSAGE_TOOKEN 0x7E

static volatile uint16_t g_uart_rx_head = 0U;
static volatile uint16_t g_uart_rx_tail = 0U;
static volatile uint16_t g_uart_rx_size = 0U;
static uint8_t g_uart_rx_ring[UART_RX_RING_SIZE] = {0U};

static uint8_t g_uart_block[UART_BLOCK_SIZE] = {0U};
static uint32_t g_uart_block_count = 0U;
static uint32_t g_uart_total_size = 0U;
static uint32_t g_uart_total_received = 0U;
static uint32_t g_uart_write_addr = 0U;
static char g_uart_cmd[UART_CMD_MAX_LEN] = {0};
static uint32_t g_uart_cmd_len = 0U;

typedef enum {
    BOOT_STATE_WAIT_CMD = 0,
    BOOT_STATE_RECV_DATA
} BootUartState;

static BootUartState g_boot_state = BOOT_STATE_WAIT_CMD;

void JumpToApplication(void);

static void DisableInterrupts(void)
{
    __asm volatile ("cpsid i");
}

/**
 * @brief Chờ một khoảng thời gian (đơn vị: millisecond)
 *        Sử dụng vòng lặp đơn giản để chờ
 * @param milliseconds: Số millisecond cần chờ
 * @note  Hàm này sử dụng con số lặp gần đúng, không chính xác tuyệt đối
 */
static void DelayMs(uint32_t milliseconds)
{
    Timer_DelayMs(milliseconds);
}

static void BootSystemReset(void)
{
    __asm volatile ("cpsid i");
    SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    while (1) {
    }
}

static void BootUartRxCallback(uint8_t byte)
{
    uint16_t next = (uint16_t)((g_uart_rx_head + 1U) % UART_RX_RING_SIZE);
    if (next == g_uart_rx_tail) {
        return;
    }
    g_uart_rx_ring[g_uart_rx_head] = byte;
    g_uart_rx_head = next;
    g_uart_rx_size++;
}

static bool BootUartPopByte(uint8_t *out_byte)
{
    if (g_uart_rx_head == g_uart_rx_tail) {
        return false;
    }
    *out_byte = g_uart_rx_ring[g_uart_rx_tail];
    g_uart_rx_tail = (uint16_t)((g_uart_rx_tail + 1U) % UART_RX_RING_SIZE);
    g_uart_rx_size--;
    return true;
}

uint16_t GetUartRingString(uint8_t *out_buf, uint16_t max_len, uint8_t token) {
    uint16_t tail = g_uart_rx_tail;
    uint16_t head = g_uart_rx_head;
    uint16_t i = 0;
    
    if (tail == head || (token != 0 && g_uart_rx_ring[tail] != token)) {
        out_buf[0] = '\0';
        return 0;
    } else {
        tail = (tail + 1) % UART_RX_RING_SIZE;
    }
    

    while (tail != head && i < (max_len - 1) && (token != 0 && g_uart_rx_ring[tail] != token)) {
        if(token != 0 && g_uart_rx_ring[tail] == token) {
            tail = (tail + 1) % UART_RX_RING_SIZE;
            g_uart_rx_size--;
            break;
        }
        out_buf[i++] = g_uart_rx_ring[tail];
        // g_uart_rx_ring[tail] = 0;
        tail = (tail + 1) % UART_RX_RING_SIZE;  
        g_uart_rx_size--;
    }
    if(token) {
        out_buf[i] = '\0';
        g_uart_rx_tail = tail + 1;
        g_uart_rx_size--;
    }
    
    return i;
}

uint16_t GetUartRingBytes(uint8_t *out_buf, uint16_t max_len) {
    uint16_t tail = g_uart_rx_tail;
    uint16_t head = g_uart_rx_head;
    uint16_t i = 0;
    
    if (tail == head) {
        return 0;
    }
    while (tail != head && i < (max_len - 1)) {
        out_buf[i++] = g_uart_rx_ring[tail];
        tail = (tail + 1) % UART_RX_RING_SIZE;  
        g_uart_rx_size--;
    }
    g_uart_rx_tail = tail;
    return i;
}


// Kiểm tra xem ring buffer UART có data không

static bool BootUartHasData(void)
{
    return (g_uart_rx_head != g_uart_rx_tail);
}

// Xóa sạch dữ liệu trong ring buffer UART
static void ClearUartRingBuffer(void)
{
    g_uart_rx_head = 0U;
    g_uart_rx_tail = 0U;
    g_uart_rx_size = 0U;
    for (uint16_t i = 0; i < UART_RX_RING_SIZE; ++i) {
        g_uart_rx_ring[i] = 0U;
    }
}

static void BootUartResetTransfer(void)
{
    g_uart_block_count = 0U;
    g_uart_total_received = 0U;
    g_uart_total_size = 0U;
    g_uart_write_addr = 0U;
    g_boot_state = BOOT_STATE_WAIT_CMD;
    g_uart_cmd_len = 0U;
}

static const char *BootUartSkipSpaces(const char *ptr)
{
    while (*ptr == ' ' || *ptr == '\t') {
        ++ptr;
    }
    return ptr;
}

static bool BootUartParseUint32(const char *ptr, uint32_t *out_value, const char **out_end)
{
    uint32_t value = 0U;
    uint32_t base = 10U;
    if ((ptr[0] == '0') && (ptr[1] == 'x' || ptr[1] == 'X')) {
        base = 16U;
        ptr += 2;
    }

    if ((*ptr == '\0') || (*ptr == ' ') || (*ptr == '\t')) {
        return false;
    }

    while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t' && *ptr != '\r' && *ptr != '\n') {
        uint32_t digit = 0xFFFFFFFFU;
        if (*ptr >= '0' && *ptr <= '9') {
            digit = (uint32_t)(*ptr - '0');
        } else if (*ptr >= 'a' && *ptr <= 'f') {
            digit = 10U + (uint32_t)(*ptr - 'a');
        } else if (*ptr >= 'A' && *ptr <= 'F') {
            digit = 10U + (uint32_t)(*ptr - 'A');
        } else {
            return false;
        }

        if (digit >= base) {
            return false;
        }

        value = (value * base) + digit;
        ++ptr;
    }

    *out_value = value;
    *out_end = ptr;
    return true;
}

// Convert uint32_t to string (decimal or hex)
// base = 10 for decimal, 16 for hex
static void Uint32ToStr(uint32_t value, char *buf, int base)
{
    char tmp[16];
    int i = 0;
    if (base == 16) {
        // Hex prefix
        buf[0] = '0';
        buf[1] = 'x';
        buf += 2;
    }
    if (value == 0) {
        *buf++ = '0';
        *buf = '\0';
        return;
    }
    while (value > 0) {
        uint32_t digit = value % base;
        if (digit < 10)
            tmp[i++] = '0' + digit;
        else
            tmp[i++] = 'A' + (digit - 10);
        value /= base;
    }
    // Reverse
    while (i > 0) {
        *buf++ = tmp[--i];
    }
    *buf = '\0';
}

static bool BootUartValidateRange(uint32_t start_addr, uint32_t size)
{
    if (size == 0U) {
        return false;
    }

    if ((start_addr & 0x3U) != 0U) {
        return false;
    }

    if (start_addr < APP_START_ADDRESS) {
        return false;
    }

    if (start_addr < BOOTLOADER_START_ADDRESS) {
        return false;
    }

    if ((start_addr - BOOTLOADER_START_ADDRESS) + size > FLASH_MAX_RANGE) {
        return false;
    }

    if ((start_addr + size) < start_addr) {
        return false;
    }

    return true;
}

static void BootUartEraseRange(uint32_t start_addr, uint32_t size)
{
    flash_unlock();
    uint32_t addr = start_addr & ~(FLASH_PAGE_SIZE - 1U);
    uint32_t end_addr = start_addr + size;
    while (addr < end_addr) {
        flash_erase_page(addr);
        addr += FLASH_PAGE_SIZE;
    }
    flash_lock();
}

static void BootUartWriteBlock(uint32_t addr, const uint8_t *data, uint32_t length)
{
    flash_unlock();
    uint32_t offset = 0U;
    while (offset < length) {
        uint32_t word = 0xFFFFFFFFU;
        uint32_t remaining = length - offset;
        uint32_t chunk = (remaining >= 4U) ? 4U : remaining;

        for (uint32_t i = 0U; i < chunk; ++i) {
            word &= ~(0xFFU << (i * 8U));
            word |= ((uint32_t)data[offset + i] << (i * 8U));
        }

        flash_write_word(addr + offset, word);
        offset += chunk;
    }
    flash_lock();
}

static void BootUartStartFlash(uint32_t start_addr, uint32_t size)
{
    // Chỉ lưu thông tin, chưa unlock/erase flash, sẽ thực hiện khi nhận đủ 1KB đầu tiên
    g_uart_total_size = size;
    g_uart_total_received = 0U;
    g_uart_block_count = 0U;
    g_uart_write_addr = start_addr;
    g_boot_state = BOOT_STATE_RECV_DATA;
    UART_SendString(UART_PORT1, k_uart_ready);
}

static void BootUartHandleCommand(const char *cmd)
{
    const char *ptr = BootUartSkipSpaces(cmd);

    // FLASH <start_addr> <size> <bin_size>
    if ((ptr[0] == 'F') && (ptr[1] == 'L') && (ptr[2] == 'A') && (ptr[3] == 'S') && (ptr[4] == 'H')) {
        ptr += 5;
        ptr = BootUartSkipSpaces(ptr);

        uint32_t start_addr = 0U;
        uint32_t size = 0U;
        const char *end = NULL;
        
        if (!BootUartParseUint32(ptr, &start_addr, &end)) {
            UART_SendString(UART_PORT1, k_uart_err);
            return;
        }

        ptr = BootUartSkipSpaces(end);
        if (!BootUartParseUint32(ptr, &size, &end)) {
            UART_SendString(UART_PORT1, k_uart_err);
            return;
        }

        if (!BootUartValidateRange(start_addr, size)) {
            UART_SendString(UART_PORT1, k_uart_err);
            return;
        }
        // Lưu thêm bin_size nếu cần sử dụng sau này
        g_uart_total_size = size;
        BootUartStartFlash(start_addr, size);
        return;
    }

    UART_SendString(UART_PORT1, k_uart_err);
}

static void BootUartProcess(void)
{
    uint16_t size_package = 0U;
    // if ((g_boot_state == BOOT_STATE_WAIT_CMD) && ((GPIOA_IDR & GPIOA0) != 0U)) {
    //     JumpToApplication();
    //     return;
    // }

    static uint8_t pkg_buf[UART_BLOCK_SIZE];

    while (BootUartHasData()) {
        DelayMs(100);
        

        if (g_boot_state == BOOT_STATE_WAIT_CMD) {
            size_package = GetUartRingString(pkg_buf, sizeof(pkg_buf), MESSAGE_TOOKEN); // Lấy toàn bộ data hiện có trong ring buffer để debug
            // UART_SendString(UART_PORT1, "DBG:RECEIVED=");
            // UART_SendString(UART_PORT1, pkg_buf);
            if(size_package > 0U) {
                BootUartHandleCommand(pkg_buf);   
            }
            continue;
        }

        // Khi đã chuyển sang chế độ nhận data
        if (g_boot_state == BOOT_STATE_RECV_DATA ) {
            int remain = g_uart_total_size - g_uart_total_received;
            uint16_t write_len = 0;
            if(remain >= UART_BLOCK_SIZE) {
                write_len = UART_BLOCK_SIZE;
            } else {
                write_len = (uint16_t)remain;
            }

            if(g_uart_rx_size >= write_len) {
                size_package = GetUartRingBytes(pkg_buf, UART_BLOCK_SIZE + 1); // Lấy toàn bộ data hiện có trong ring buffer để debug
                if(size_package == 0U) {
                    continue;
                }
            } else {
                continue;
            }

            // Uint32ToStr(size_package, debug_buf, 10);
            // UART_SendString(UART_PORT1, "DBG:RECEIVED=");
            // UART_SendString(UART_PORT1, debug_buf);
            // UART_SendString(UART_PORT1, "\n");
            DelayMs(10);

            if (g_uart_total_received < g_uart_total_size) {

                
                write_len = (size_package < write_len) ? size_package : write_len;  

                Uint32ToStr(write_len, debug_buf, 10);
                UART_SendString(UART_PORT1, "DBG:WRITE_LEN=");
                UART_SendString(UART_PORT1, debug_buf);
                UART_SendString(UART_PORT1, "\n");
                DelayMs(10);

                if (write_len > 0 && write_len <= UART_BLOCK_SIZE) {

                    // UART_SendString(UART_PORT1, "DBG:CMD_RECEIVED=");
                    // UART_SendBytes(UART_PORT1, pkg_buf, write_len);
                    // UART_SendString(UART_PORT1, "\n");
                    // DelayMs(10);
                    BootUartEraseRange(g_uart_write_addr, write_len);
                    BootUartWriteBlock(g_uart_write_addr, pkg_buf, write_len);
                    g_uart_write_addr += write_len;
                    g_uart_total_received += write_len;

                    Uint32ToStr(g_uart_total_received, debug_buf, 10);
                    UART_SendString(UART_PORT1, "DBG:g_uart_total_received=");
                    UART_SendString(UART_PORT1, debug_buf);
                    UART_SendString(UART_PORT1, "\n");
                    DelayMs(10);

                    if (g_uart_total_received >= g_uart_total_size) {
                    // flash_lock();
                    ClearUartRingBuffer();
                    UART_SendString(UART_PORT1, k_uart_done);
                    DelayMs(10);
                    BootUartResetTransfer();
                    // JumpToApplication();
                    BootSystemReset();
                }
                    // UART_SendString(UART_PORT1, k_uart_ack);
                    // DelayMs(10);
                } else {
                    // Uint32ToStr(write_len, debug_buf, 10);
                    // UART_SendString(UART_PORT1, "DBG:WRITE_LEN=");
                    // UART_SendString(UART_PORT1, debug_buf);
                    // UART_SendString(UART_PORT1, "\n");
                    // DelayMs(10);
                    UART_SendString(UART_PORT1, k_uart_err);
                    DelayMs(10);
                }
            }
        }
    }
}




//#################################################     CODE      ###############################################

bool memcmp_array(const void *ptr1, const void *ptr2, size_t num) {
    const uint8_t *b1 = (const uint8_t *)ptr1;
    const uint8_t *b2 = (const uint8_t *)ptr2;
    uint8_t debug_buf[32];
    for (size_t i = 0; i < num; ++i) {
        if (b1[i] != b2[i]) {
            return false;
        }
    }
    return true;
}

static bool VerifyImage(uint32_t address, uint32_t size)
{
    uint8_t stored_hash[32];
    for(uint8_t i = 0; i < 32U; ++i) {
        stored_hash[i] = *(const volatile uint8_t *)(address + size - 36 + i);
    }

    const uint8_t *firmware = (const volatile uint8_t *)(address);

    const uint32_t stored_crc = *(const volatile uint32_t *)(address + size - 4);
    Sha256Ctx ctx;
    uint8_t digest[32];
    uint8_t chunk[128];
    uint32_t offset = 0U;
    uint32_t crc;
    uint8_t debug_buf[32];

    uint32_t firmware_size = *(const volatile uint32_t *)(address + size - 40);

    // UART_SendString(UART_PORT1, "firmware_size:");
    // Uint32ToStr(firmware_size, debug_buf, 16);
    // UART_SendString(UART_PORT1, debug_buf);
    // UART_SendString(UART_PORT1, "\n");

    sha256_init(&ctx);
    sha256_update(&ctx, firmware, firmware_size);
    crc = crc32_calc(0xFFFFFFFFU, firmware, firmware_size);
    

    uint8_t *crc_buf = &crc;
    uint8_t *store_crc_buf = &stored_crc;

    sha256_final(&ctx, digest);
    crc = crc32_finalize(crc);

    //     UART_SendString(UART_PORT1, "SHA:");
    // for(uint8_t i = 0; i < 32; i++) {
    //     Uint32ToStr((uint8_t)digest[i], debug_buf, 16);
    //     UART_SendString(UART_PORT1, debug_buf);
    // }
    // UART_SendString(UART_PORT1, "\n");
    // UART_SendString(UART_PORT1, "SHA store:");
    // for(uint8_t i = 0; i < 32; i++) {
    //     Uint32ToStr((uint8_t)stored_hash[i], debug_buf, 16);
    //     UART_SendString(UART_PORT1, debug_buf);
    // }
    // UART_SendString(UART_PORT1, "\n");

    // UART_SendString(UART_PORT1, "CRC:");
    // for(uint8_t i = 0; i < 4; i++) {
    //     Uint32ToStr((uint8_t)crc_buf[i], debug_buf, 16);
    //     UART_SendString(UART_PORT1, debug_buf);
    // }
    // UART_SendString(UART_PORT1, "\n");
    // UART_SendString(UART_PORT1, "CRC store:");
    // for(uint8_t i = 0; i < 4; i++) {
    //     Uint32ToStr((uint8_t)store_crc_buf[i], debug_buf, 16);
    //     UART_SendString(UART_PORT1, debug_buf);
    // }
    // UART_SendString(UART_PORT1, "\n");

    // Uint32ToStr(crc, debug_buf, 16);
    // UART_SendString(UART_PORT1, debug_buf);
    return (memcmp_array(digest, stored_hash, 32) && (crc == stored_crc));
}


// Function prototype for jumping to the application
void JumpToApplication(void) {
    // Get the application's initial MSP and reset handler from its vector table
    uint32_t sp = *(volatile uint32_t*)APP_START_ADDRESS;
    uint32_t reset = *(volatile uint32_t*)(APP_START_ADDRESS + 4);

    // Validate the application image before jumping
    if ((reset & 1U) == 0U) {
        return;
    }

    // Disable interrupts before jumping to application
    DisableInterrupts();

    // Relocate Vector Table to the application's vector table
    SCB_VTOR = APP_START_ADDRESS;

    // Set the stack pointer to the application's stack pointer
    __asm volatile ("msr msp, %0" : : "r" (sp));

    // Jump to the application's reset handler
    void (*ApplicationEntryPoint)(void) = (void (*)(void))reset;
    ApplicationEntryPoint();
}

void JumpToOta(void) {
    // Get the application's initial MSP and reset handler from its vector table
    uint32_t sp = *(volatile uint32_t*)APP_OTA_START_ADDRESS;
    uint32_t reset = *(volatile uint32_t*)(APP_OTA_START_ADDRESS + 4);

    // Validate the application image before jumping
    if ((reset & 1U) == 0U) {
        return;
    }

    // Disable interrupts before jumping to application
    DisableInterrupts();

    // Relocate Vector Table to the application's vector table
    SCB_VTOR = APP_OTA_START_ADDRESS;

    // Set the stack pointer to the application's stack pointer
    __asm volatile ("msr msp, %0" : : "r" (sp));

    // Jump to the application's reset handler
    void (*ApplicationOTAEntryPoint)(void) = (void (*)(void))reset;
    ApplicationOTAEntryPoint();
}

// Bootloader entry point
void BootLoaderEntryPoint(void) {
    /**
     * Bootloader Entry Point - Quy trình khởi động
     * 
     * Quy trình:
     */
    
    // Configure system clock to 72MHz before initializing peripherals
    (void)Frequency_Apply(FREQUENCY_72MHZ);

    // // Initialize TIM2 to provide 1ms delay helper
    (void)Timer_InitTIM2_1MHz();

    UART_Init(UART_PORT1, 115200U);
    //UART_SendBytes(UART_PORT1, k_uart_greeting, sizeof(k_uart_greeting) - 1U);
    UART_EnableRxInterrupt(UART_PORT1, BootUartRxCallback);
    __asm volatile ("cpsie i");

    // Enable GPIOA clock and set PA0 input pull-up
    RCC_APB2ENR |= RCC_IOPAEN;
    GPIOA_CRL &= ~0x0000000FU;
    GPIOA_CRL |= 0x44;
    GPIOA_ODR |= GPIOA0;
    GPIOA_ODR |= GPIOA1;



    
    // RCC_APB2ENR |= RCC_IOPCEN;
    // GPIOC_CRH &= 0xFF0FFFFF;
    // GPIOC_CRH |= 0x00200000;

    // Enable GPIOC clock (APB2ENR, bit 4)
    RCC_APB2ENR |= RCC_IOPCEN;
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;

    GPIOC_ODR |= GPIOC13;

        // Verify app image hash before jumping
    
    while (1)
    {
        if((GPIOA_IDR & GPIOA0) == 0U) {
            BootUartProcess();
            DelayMs(10);
        } else {
            if ((GPIOA_IDR & GPIOA1)) {
                if(VerifyImage(APP_START_ADDRESS, APP_IMAGE_SIZE)) {
                    UART_SendString(UART_PORT1, "APP image valid, jumping to application...\n");
                    JumpToApplication();
                } else if (VerifyImage(APP_OTA_START_ADDRESS, APP_IMAGE_SIZE))
                {
                    UART_SendString(UART_PORT1, "APP image invalid, jumping to OTA...\n");
                    JumpToOta();
                }
            } else {
                if(VerifyImage(APP_OTA_START_ADDRESS, APP_IMAGE_SIZE)) {
                    UART_SendString(UART_PORT1, "OTA image valid, jumping to APP...\n");
                    JumpToOta();
                } else if (VerifyImage(APP_START_ADDRESS, APP_IMAGE_SIZE))
                {
                    UART_SendString(UART_PORT1, "APP image invalid, jumping to OTA...\n");
                    JumpToOta();
                }
            }
            //JumpToApplication();
        }
    }
}

