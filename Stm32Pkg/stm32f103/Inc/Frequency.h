/**
 * @file Frequency.h
 * @brief Predefined clock configurations for STM32F103C8T6 (HSE=8MHz)
 */
#ifndef __FREQUENCY_H
#define __FREQUENCY_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FREQUENCY_32MHZ = 0,
    FREQUENCY_48MHZ,
    FREQUENCY_56MHZ,
    FREQUENCY_64MHZ,
    FREQUENCY_72MHZ,
} Frequency_Profile_t;

bool Frequency_Apply(Frequency_Profile_t profile);

uint32_t Frequency_GetSYSCLK(Frequency_Profile_t profile);
uint32_t Frequency_GetHCLK(Frequency_Profile_t profile);
uint32_t Frequency_GetPCLK1(Frequency_Profile_t profile);
uint32_t Frequency_GetPCLK2(Frequency_Profile_t profile);

#endif /* __FREQUENCY_H */
