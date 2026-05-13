/**
 * @file Timer.c
 * @brief TIM2-based millisecond delay helper for STM32F103
 *
 * The timer is configured as a free-running 1MHz counter.
 * One tick = 1 microsecond, so 1000 ticks = 1 millisecond.
 */

#include "Timer.h"

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RESERVED1;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t RESERVED2;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

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

#define RCC_BASE        ((uint32_t)0x40021000UL)
#define TIM2_BASE       ((uint32_t)0x40000000UL)
#define RCC             ((RCC_TypeDef *)RCC_BASE)
#define TIM2            ((TIM_TypeDef *)TIM2_BASE)

#define RCC_APB1ENR_TIM2EN (1U << 0)

#define TIM_CR1_CEN     (1U << 0)
#define TIM_EGR_UG      (1U << 0)

static bool g_timer_initialized = false;

bool Timer_InitTIM2_1MHz(void)
{
    /* Enable TIM2 clock on APB1. */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* Stop timer while configuring. */
    TIM2->CR1 &= ~TIM_CR1_CEN;

    /*
     * TIM2 input clock on STM32F103:
     * - If APB1 prescaler = 2, timer clock = 2 * PCLK1 = 72MHz.
     * With PSC = 71, we get 1MHz counter clock.
     */
    TIM2->PSC = 71U;
    TIM2->ARR = 0xFFFFFFFFUL;
    TIM2->CNT = 0U;
    TIM2->EGR = TIM_EGR_UG;

    /* Enable free running counter. */
    TIM2->CR1 |= TIM_CR1_CEN;
    g_timer_initialized = true;
    return true;
}

void Timer_DelayMs(uint32_t milliseconds)
{
    if (!g_timer_initialized) {
        (void)Timer_InitTIM2_1MHz();
    }

    while (milliseconds > 0U) {
        uint32_t start = TIM2->CNT;
        while ((uint32_t)(TIM2->CNT - start) < 1000U) {
            __asm__("nop");
        }
        milliseconds--;
    }
}
