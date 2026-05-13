/*****************************************************************************************************************
 * @Filename: ApplicationEntryPoint.c
 * @Author: Đinh Văn Tuấn
 * @Date 2026-05-01
 * @Version: 1.0
 * @Description: Main Application
 * 
 * Copyright (c) 2026 []. All rights reserved.
 * Licensed under the MIT License.
 ****************************************************************************************************************/


//################################################# INCLUDE HEARDER #############################################
#include <stdint.h>
#include <ProjectConfig.h>
#include "../../boot_flag.h"

//#################################################     DEFINE    ###############################################
// #define RCC_APB2ENR (*(volatile uint32_t*)0x40021018U)
// #define GPIOC_CRH   (*(volatile uint32_t*)0x40011004U)
// #define GPIOC_ODR   (*(volatile uint32_t*)0x4001100CU)

#define LED_PIN     13U

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
#define GPIOC13 (1UL << 13)
#define GPIOA0 (1UL << 0)

//#################################################     TYPEDEF   ###############################################


//#################################################     VARIABLE  ###############################################


//#################################################     CODE      ###############################################

static void AppSystemReset(void)
{
    __asm volatile ("cpsid i");
    SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    while (1) {
    }
}

// static void delay_ms(uint32_t count)
// {
//     while (count--)
//     {
//         for (volatile uint32_t i = 0; i < 8000U; i++)
//         {
//             __asm volatile ("nop");
//         }
//     }
// }

// typedef struct {
//     uint32_t magic;
//     uint32_t version;
//     char version_str[16];
// } user_data_t;

// const user_data_t app_info __attribute__((section(".user_data"), used)) =
// {
//     .magic = 0xA5A5A5A5,
//     .version = 0x00010000,
//     .version_str = "v1.0.0"
// };

// extern const uint32_t _suser_data;
// const user_data_t *info = (const user_data_t *)&_suser_data;

void JumpToOta(void) {
    // Get the application's initial MSP and reset handler from its vector table
    uint32_t sp = *(volatile uint32_t*)APP_OTA_START_ADDRESS;
    uint32_t reset = *(volatile uint32_t*)(APP_OTA_START_ADDRESS + 4);


    // Disable interrupts before jumping to application
    __asm volatile ("cpsid i");

    // Relocate Vector Table to the application's vector table
    SCB_VTOR = APP_OTA_START_ADDRESS;

    // Set the stack pointer to the application's stack pointer
    __asm volatile ("msr msp, %0" : : "r" (sp));

    // Jump to the application's reset handler
    void (*ApplicationOTAEntryPoint)(void) = (void (*)(void))reset;
    ApplicationOTAEntryPoint();
}

void ApplicationEntryPoint(void)
{
    

    // Enable GPIOC and GPIOA clocks (APB2ENR)
    RCC_APB2ENR |= (RCC_IOPCEN | RCC_IOPAEN);
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;
    uint32_t counter = 0;

    // PA0 input with pull-up: MODE0=00, CNF0=10, ODR=1
    GPIOA_CRL &= ~0x0000000FU;
    GPIOA_CRL |= 0x00000008U;
    GPIOA_ODR |= GPIOA0;

    while (1)
    {
        GPIOC_ODR |= GPIOC13;
        for (int i = 0; i < 500000; i++)
            ; // arbitrary delay
        GPIOC_ODR &= ~GPIOC13;
        for (int i = 0; i < 500000; i++)
            ; // arbitrary delay
        counter++;

        if ((GPIOA_IDR & GPIOA0) == 0U) {
            //boot_flag_request_flash(APP_START_ADDRESS, APP_IMAGE_SIZE);
            AppSystemReset();
        }

        // if ((boot_flag_get() == BOOT_FLAG_BOOTLOADER) && (boot_flag_get_cmd() == BOOT_CMD_FLASH)) {
        //     AppSystemReset();
        // }
    }
    //JumpToOta();
}
