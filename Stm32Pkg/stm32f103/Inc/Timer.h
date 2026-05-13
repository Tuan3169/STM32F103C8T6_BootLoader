/**
 * @file Timer.h
 * @brief Minimal millisecond timer support using TIM2 on STM32F103
 */
#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>
#include <stdbool.h>

bool Timer_InitTIM2_1MHz(void);
void Timer_DelayMs(uint32_t milliseconds);

#endif /* __TIMER_H */
