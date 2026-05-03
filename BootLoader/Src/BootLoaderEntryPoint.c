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
#include <ProjectConfig.h>


//#################################################     DEFINE    ###############################################
///define APP_START_ADDRESS 0x08001000
// Define SCB for Vector Table Relocation
#define SCB_BASE        0xE000ED00
#define SCB_VTOR        (*(volatile uint32_t*)(SCB_BASE + 0x08))

// register address
#define RCC_BASE 0x40021000
#define GPIOC_BASE 0x40011000

#define RCC_APB2ENR *(volatile uint32_t *)(RCC_BASE + 0x18)
#define GPIOC_CRH *(volatile uint32_t *)(GPIOC_BASE + 0x04)
#define GPIOC_ODR *(volatile uint32_t *)(GPIOC_BASE + 0x0C)

// bit fields
#define RCC_IOPCEN (1 << 4)
#define GPIOC13 (1UL << 13)

//#################################################     TYPEDEF   ###############################################


//#################################################     VARIABLE  ###############################################


//#################################################     CODE      ###############################################

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
    __asm volatile ("cpsid i");

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
    __asm volatile ("cpsid i");

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
    // Perform any necessary initialization here (e.g., hardware setup, communication, etc.)

    // Jump to the application
    JumpToApplication();
    //JumpToOta();
    // RCC_APB2ENR |= RCC_IOPCEN;
    // GPIOC_CRH &= 0xFF0FFFFF;
    // GPIOC_CRH |= 0x00200000;
    // while (1)
    // {
    //     GPIOC_ODR |= GPIOC13;
    //     for (int i = 0; i < 500000; i++)
    //         ; // arbitrary delay
    //     GPIOC_ODR &= ~GPIOC13;
    //     for (int i = 0; i < 500000; i++)
    //         ; // arbitrary delay
    // }
    // The bootloader should never return here
    while (1) {
        // Optionally, you can add a watchdog reset or error handling here
    }
}

