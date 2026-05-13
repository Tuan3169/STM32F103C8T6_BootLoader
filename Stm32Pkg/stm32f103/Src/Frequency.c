/**
 * @file Frequency.c
 * @brief Predefined clock configurations for STM32F103C8T6 (HSE=8MHz)
 */

#include "Frequency.h"
#include "SystemClock.h"
#include <stddef.h>

typedef struct {
    uint8_t pll_mul;
    uint8_t ahb_div;
    uint8_t apb1_div;
    uint8_t apb2_div;
    uint32_t sysclk_hz;
    uint32_t hclk_hz;
    uint32_t pclk1_hz;
    uint32_t pclk2_hz;
} Frequency_Config_t;

static const Frequency_Config_t kFrequencyTable[] = {
    /* pll_mul, ahb_div, apb1_div, apb2_div, sysclk, hclk, pclk1, pclk2 */
    {4U, 1U, 2U, 1U, 32000000UL, 32000000UL, 16000000UL, 32000000UL},
    {6U, 1U, 2U, 1U, 48000000UL, 48000000UL, 24000000UL, 48000000UL},
    {7U, 1U, 2U, 1U, 56000000UL, 56000000UL, 28000000UL, 56000000UL},
    {8U, 1U, 2U, 1U, 64000000UL, 64000000UL, 32000000UL, 64000000UL},
    {9U, 1U, 2U, 1U, 72000000UL, 72000000UL, 36000000UL, 72000000UL},
};

static const Frequency_Config_t *Frequency_GetConfig(Frequency_Profile_t profile)
{
    if ((uint32_t)profile >= (sizeof(kFrequencyTable) / sizeof(kFrequencyTable[0]))) {
        return NULL;
    }
    return &kFrequencyTable[(uint32_t)profile];
}

bool Frequency_Apply(Frequency_Profile_t profile)
{
    const Frequency_Config_t *cfg = Frequency_GetConfig(profile);
    if (cfg == NULL) {
        return false;
    }

    return SystemClock_ConfigHSE_PLL(cfg->pll_mul, cfg->ahb_div, cfg->apb1_div, cfg->apb2_div);
}

uint32_t Frequency_GetSYSCLK(Frequency_Profile_t profile)
{
    const Frequency_Config_t *cfg = Frequency_GetConfig(profile);
    return (cfg != NULL) ? cfg->sysclk_hz : 0U;
}

uint32_t Frequency_GetHCLK(Frequency_Profile_t profile)
{
    const Frequency_Config_t *cfg = Frequency_GetConfig(profile);
    return (cfg != NULL) ? cfg->hclk_hz : 0U;
}

uint32_t Frequency_GetPCLK1(Frequency_Profile_t profile)
{
    const Frequency_Config_t *cfg = Frequency_GetConfig(profile);
    return (cfg != NULL) ? cfg->pclk1_hz : 0U;
}

uint32_t Frequency_GetPCLK2(Frequency_Profile_t profile)
{
    const Frequency_Config_t *cfg = Frequency_GetConfig(profile);
    return (cfg != NULL) ? cfg->pclk2_hz : 0U;
}
