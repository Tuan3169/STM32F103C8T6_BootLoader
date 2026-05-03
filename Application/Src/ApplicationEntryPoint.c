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

//#################################################     DEFINE    ###############################################
// #define RCC_APB2ENR (*(volatile uint32_t*)0x40021018U)
// #define GPIOC_CRH   (*(volatile uint32_t*)0x40011004U)
// #define GPIOC_ODR   (*(volatile uint32_t*)0x4001100CU)

#define LED_PIN     13U

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
    // Enable GPIOC clock (APB2ENR, bit 4)
    RCC_APB2ENR |= RCC_IOPCEN;
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;
    uint32_t counter = 0;

    while (counter < 5)
    {
        GPIOC_ODR |= GPIOC13;
        for (int i = 0; i < 500000; i++)
            ; // arbitrary delay
        GPIOC_ODR &= ~GPIOC13;
        for (int i = 0; i < 500000; i++)
            ; // arbitrary delay
        counter++;
    }
    JumpToOta();
}
