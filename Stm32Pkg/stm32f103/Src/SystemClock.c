/**
 * @file SystemClock.c
 * @brief Minimal register-level clock setup for STM32F103C8T6
 *
 * This module configures:
 * - HSE = 8MHz crystal
 * - PLL = HSE x 9
 * - SYSCLK = 72MHz
 * - AHB = 72MHz
 * - APB1 = 36MHz
 * - APB2 = 72MHz
 */

#include "SystemClock.h"

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t ACR;
} FLASH_TypeDef;

#define RCC_BASE        ((uint32_t)0x40021000UL)
#define FLASH_BASE      ((uint32_t)0x40022000UL)
#define RCC             ((RCC_TypeDef *)RCC_BASE)
#define FLASH           ((FLASH_TypeDef *)FLASH_BASE)

/* RCC CR bits */
#define RCC_CR_HSION    (1U << 0)
#define RCC_CR_HSIRDY   (1U << 1)
#define RCC_CR_HSEON    (1U << 16)
#define RCC_CR_HSERDY   (1U << 17)
#define RCC_CR_HSEBYP   (1U << 18)
#define RCC_CR_PLLON    (1U << 24)
#define RCC_CR_PLLRDY   (1U << 25)

/* RCC CFGR bits */
#define RCC_CFGR_SW_Pos         0U
#define RCC_CFGR_SW_MASK        (3U << RCC_CFGR_SW_Pos)
#define RCC_CFGR_SW_HSE         (1U << 0)
#define RCC_CFGR_SW_PLL         (2U << 0)

#define RCC_CFGR_SWS_Pos        2U
#define RCC_CFGR_SWS_MASK       (3U << RCC_CFGR_SWS_Pos)

#define RCC_CFGR_HPRE_Pos       4U
#define RCC_CFGR_PPRE1_Pos      8U
#define RCC_CFGR_PPRE2_Pos      11U

#define RCC_CFGR_HPRE_DIV1      (0U << RCC_CFGR_HPRE_Pos)
#define RCC_CFGR_PPRE1_DIV2     (4U << RCC_CFGR_PPRE1_Pos)
#define RCC_CFGR_PPRE2_DIV1     (0U << RCC_CFGR_PPRE2_Pos)

#define RCC_CFGR_PLLSRC         (1U << 16)
#define RCC_CFGR_PLLXTPRE       (1U << 17)
#define RCC_CFGR_PLLMUL_Pos     18U

#define FLASH_ACR_LATENCY_MASK  (7U << 0)
#define FLASH_ACR_PRFTBE        (1U << 4)

/* Computed clock values after configuration */
static uint32_t g_sysclk_hz = 8000000UL;
static uint32_t g_hclk_hz = 8000000UL;
static uint32_t g_pclk1_hz = 8000000UL;
static uint32_t g_pclk2_hz = 8000000UL;

static void wait_for_flag(volatile uint32_t *reg, uint32_t mask, bool set)
{
    for (volatile uint32_t timeout = 0; timeout < 1000000UL; ++timeout) {
        if (set) {
            if ((*reg & mask) != 0U) {
                return;
            }
        } else {
            if ((*reg & mask) == 0U) {
                return;
            }
        }
    }
}

static bool SystemClock_EncodeAHBPrescaler(uint8_t div, uint32_t *bits)
{
    switch (div) {
        case 1: *bits = 0x0U << RCC_CFGR_HPRE_Pos; return true;
        case 2: *bits = 0x8U << RCC_CFGR_HPRE_Pos; return true;
        case 4: *bits = 0x9U << RCC_CFGR_HPRE_Pos; return true;
        case 8: *bits = 0xAU << RCC_CFGR_HPRE_Pos; return true;
        case 16: *bits = 0xBU << RCC_CFGR_HPRE_Pos; return true;
        case 64: *bits = 0xCU << RCC_CFGR_HPRE_Pos; return true;
        case 128: *bits = 0xDU << RCC_CFGR_HPRE_Pos; return true;
        default: return false;
    }
}

static bool SystemClock_EncodeAPBPrescaler(uint8_t div, uint32_t pos, uint32_t *bits)
{
    switch (div) {
        case 1: *bits = 0x0U << pos; return true;
        case 2: *bits = 0x4U << pos; return true;
        case 4: *bits = 0x5U << pos; return true;
        case 8: *bits = 0x6U << pos; return true;
        case 16: *bits = 0x7U << pos; return true;
        default: return false;
    }
}

static bool SystemClock_EncodePLLMul(uint8_t pll_mul, uint32_t *bits)
{
    if (pll_mul < 2 || pll_mul > 16) {
        return false;
    }
    /* Encoding: 2->0, 3->1, ..., 16->14 */
    *bits = (uint32_t)(pll_mul - 2U) << RCC_CFGR_PLLMUL_Pos;
    return true;
}

static void SystemClock_ConfigFlashLatency(uint32_t hclk_hz)
{
    uint32_t latency = 0U;

    if (hclk_hz > 48000000UL) {
        latency = 2U;
    } else if (hclk_hz > 24000000UL) {
        latency = 1U;
    }

    uint32_t acr = FLASH->ACR;
    acr &= ~FLASH_ACR_LATENCY_MASK;
    acr |= latency;
    acr |= FLASH_ACR_PRFTBE;
    FLASH->ACR = acr;
}

bool SystemClock_ConfigHSE_PLL(uint8_t pll_mul, uint8_t ahb_div, uint8_t apb1_div, uint8_t apb2_div)
{
    uint32_t hpre_bits = 0U;
    uint32_t ppre1_bits = 0U;
    uint32_t ppre2_bits = 0U;
    uint32_t pll_bits = 0U;

    if (!SystemClock_EncodeAHBPrescaler(ahb_div, &hpre_bits)) {
        return false;
    }
    if (!SystemClock_EncodeAPBPrescaler(apb1_div, RCC_CFGR_PPRE1_Pos, &ppre1_bits)) {
        return false;
    }
    if (!SystemClock_EncodeAPBPrescaler(apb2_div, RCC_CFGR_PPRE2_Pos, &ppre2_bits)) {
        return false;
    }
    if (!SystemClock_EncodePLLMul(pll_mul, &pll_bits)) {
        return false;
    }

    uint32_t target_sysclk = 8000000UL * (uint32_t)pll_mul;
    uint32_t target_hclk = target_sysclk / (uint32_t)ahb_div;
    SystemClock_ConfigFlashLatency(target_hclk);

    /* Start from a known state: enable HSI so firmware has a stable fallback. */
    RCC->CR |= RCC_CR_HSION;
    wait_for_flag(&RCC->CR, RCC_CR_HSIRDY, true);

    /* Enable HSE and wait until ready. */
    RCC->CR |= RCC_CR_HSEON;
    wait_for_flag(&RCC->CR, RCC_CR_HSERDY, true);
    if ((RCC->CR & RCC_CR_HSERDY) == 0U) {
        return false;
    }

    /* Disable PLL before reconfiguration. */
    RCC->CR &= ~RCC_CR_PLLON;
    wait_for_flag(&RCC->CR, RCC_CR_PLLRDY, false);

    /* Configure prescalers and PLL (HSE as PLL source, no predivider). */
    RCC->CFGR = (RCC->CFGR & ~(
        (0xFU << RCC_CFGR_HPRE_Pos) |
        (0x7U << RCC_CFGR_PPRE1_Pos) |
        (0x7U << RCC_CFGR_PPRE2_Pos) |
        (3U << RCC_CFGR_PLLMUL_Pos) |
        RCC_CFGR_PLLSRC |
        RCC_CFGR_PLLXTPRE)) |
        hpre_bits |
        ppre1_bits |
        ppre2_bits |
        RCC_CFGR_PLLSRC |
        pll_bits;

    /* Select HSE as PLL source, no HSE predivider. */
    RCC->CFGR |= RCC_CFGR_PLLSRC;
    RCC->CFGR &= ~RCC_CFGR_PLLXTPRE;

    /* Enable PLL and wait ready. */
    RCC->CR |= RCC_CR_PLLON;
    wait_for_flag(&RCC->CR, RCC_CR_PLLRDY, true);
    if ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
        return false;
    }

    /* Switch SYSCLK to PLL and wait until the switch is complete. */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_PLL;
    for (volatile uint32_t timeout = 0; timeout < 1000000UL; ++timeout) {
        if ((RCC->CFGR & RCC_CFGR_SWS_MASK) == (2U << RCC_CFGR_SWS_Pos)) {
            /* Update cached frequencies after successful switch. */
            g_sysclk_hz = 8000000UL * (uint32_t)pll_mul;
            g_hclk_hz = g_sysclk_hz / (uint32_t)ahb_div;
            g_pclk1_hz = g_hclk_hz / (uint32_t)apb1_div;
            g_pclk2_hz = g_hclk_hz / (uint32_t)apb2_div;
            return true;
        }
    }

    return false;
}

bool SystemClock_Config72MHz(void)
{
    return SystemClock_ConfigHSE_PLL(9U, 1U, 2U, 1U);
}

uint32_t SystemCoreClock_get(void)
{
    return g_sysclk_hz;
}

uint32_t SystemHCLK_get(void)
{
    return g_hclk_hz;
}

uint32_t SystemPCLK1_get(void)
{
    return g_pclk1_hz;
}

uint32_t SystemPCLK2_get(void)
{
    return g_pclk2_hz;
}
