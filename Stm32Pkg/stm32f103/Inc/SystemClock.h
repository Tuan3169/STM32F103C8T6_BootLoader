/**
 * @file SystemClock.h
 * @brief System clock configuration for STM32F103C8T6 (HSE=8MHz)
 */
#ifndef __SYSTEM_CLOCK_H
#define __SYSTEM_CLOCK_H

#include <stdint.h>
#include <stdbool.h>

/* Generic HSE + PLL configuration. Returns true on success. */
bool SystemClock_ConfigHSE_PLL(uint8_t pll_mul, uint8_t ahb_div, uint8_t apb1_div, uint8_t apb2_div);

/* Convenience function for 72MHz (HSE=8MHz, PLL x9). */
bool SystemClock_Config72MHz(void);

/* Runtime getters for computed clocks (in Hz). */
uint32_t SystemCoreClock_get(void); /* SYSCLK */
uint32_t SystemHCLK_get(void);      /* HCLK / AHB */
uint32_t SystemPCLK1_get(void);     /* PCLK1 / APB1 */
uint32_t SystemPCLK2_get(void);     /* PCLK2 / APB2 */

#endif /* __SYSTEM_CLOCK_H */
