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

void ApplicationOTAEntryPoint(void)
{
    // Enable GPIOC clock (APB2ENR, bit 4)
    RCC_APB2ENR |= RCC_IOPCEN;
    GPIOC_CRH &= 0xFF0FFFFF;
    GPIOC_CRH |= 0x00200000;

    GPIOC_ODR |= GPIOC13;
    for (int i = 0; i < 1000000; i++)
        ; // arbitrary delay

    uint32_t count = 0;

    while (count < 10)
    {
        GPIOC_ODR |= GPIOC13;
        for (int i = 0; i < 100000; i++)
            ; // arbitrary delay
        GPIOC_ODR &= ~GPIOC13;
        for (int i = 0; i < 100000; i++)
            ; // arbitrary delay
        count++;
    }

}
