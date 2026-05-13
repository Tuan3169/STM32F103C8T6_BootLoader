/**
 * @file spi_regs.h
 * @brief SPI register-level support for STM32F103 (SPI1 / SPI2)
 *
 * This header provides a small register-level SPI API that works for
 * SPI1 and SPI2 on the STM32F103C8T6.
 */

#ifndef __SPI_REGS_H
#define __SPI_REGS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPI_INSTANCE_1 = 1,
    SPI_INSTANCE_2 = 2,
} SPI_Instance_t;

#define SPI_MODE_0 0
#define SPI_MODE_1 1
#define SPI_MODE_2 2
#define SPI_MODE_3 3

#define SPI_BAUDRATEPRESCALER_2   (0x0U << 3)
#define SPI_BAUDRATEPRESCALER_4   (0x1U << 3)
#define SPI_BAUDRATEPRESCALER_8   (0x2U << 3)
#define SPI_BAUDRATEPRESCALER_16  (0x3U << 3)
#define SPI_BAUDRATEPRESCALER_32  (0x4U << 3)
#define SPI_BAUDRATEPRESCALER_64  (0x5U << 3)
#define SPI_BAUDRATEPRESCALER_128 (0x6U << 3)
#define SPI_BAUDRATEPRESCALER_256 (0x7U << 3)

#define SPI_DFF_8BIT  0
#define SPI_DFF_16BIT (1U << 11)

#define SPI_CPOL_LOW   0
#define SPI_CPOL_HIGH  (1U << 1)
#define SPI_CPHA_1EDGE 0
#define SPI_CPHA_2EDGE (1U << 0)

bool SPI_InitMaster(SPI_Instance_t instance, uint32_t baud_prescaler, uint8_t mode);
bool SPI_Enable(SPI_Instance_t instance, bool enable);
uint8_t SPI_TransferByte(SPI_Instance_t instance, uint8_t data);
bool SPI_IsBusy(SPI_Instance_t instance);

/* DMA functions */
bool SPI_EnableDMA(SPI_Instance_t instance, bool tx_enable, bool rx_enable);
bool SPI_TransmitDMA(SPI_Instance_t instance, const uint8_t *data, uint16_t size);
bool SPI_ReceiveDMA(SPI_Instance_t instance, uint8_t *data, uint16_t size);
bool SPI_IsDMATxComplete(SPI_Instance_t instance);
bool SPI_IsDMARxComplete(SPI_Instance_t instance);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_REGS_H */
