/**
 * @file spi_regs.c
 * @brief Register-level SPI implementation for STM32F103 (SPI1 / SPI2)
 */

#include "spi_regs.h"
#include <stddef.h>

/* Minimal peripheral register definitions for STM32F103 */

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
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
    volatile uint16_t CR1;
    uint16_t RESERVED0;
    volatile uint16_t CR2;
    uint16_t RESERVED1;
    volatile uint16_t SR;
    uint16_t RESERVED2;
    volatile uint16_t DR;
    uint16_t RESERVED3;
    volatile uint16_t CRCPR;
    uint16_t RESERVED4;
    volatile uint16_t RXCRCR;
    uint16_t RESERVED5;
    volatile uint16_t TXCRCR;
    uint16_t RESERVED6;
    volatile uint16_t I2SCFGR;
    uint16_t RESERVED7;
    volatile uint16_t I2SPR;
    uint16_t RESERVED8;
} SPI_TypeDef;

typedef struct {
    volatile uint32_t ISR;
    volatile uint32_t IFCR;
    volatile uint32_t CCR1;
    volatile uint32_t CNDTR1;
    volatile uint32_t CPAR1;
    volatile uint32_t CMAR1;
    uint32_t RESERVED0;
    volatile uint32_t CCR2;
    volatile uint32_t CNDTR2;
    volatile uint32_t CPAR2;
    volatile uint32_t CMAR2;
    uint32_t RESERVED1;
    volatile uint32_t CCR3;
    volatile uint32_t CNDTR3;
    volatile uint32_t CPAR3;
    volatile uint32_t CMAR3;
    uint32_t RESERVED2;
    volatile uint32_t CCR4;
    volatile uint32_t CNDTR4;
    volatile uint32_t CPAR4;
    volatile uint32_t CMAR4;
    uint32_t RESERVED3;
    volatile uint32_t CCR5;
    volatile uint32_t CNDTR5;
    volatile uint32_t CPAR5;
    volatile uint32_t CMAR5;
    uint32_t RESERVED4;
    volatile uint32_t CCR6;
    volatile uint32_t CNDTR6;
    volatile uint32_t CPAR6;
    volatile uint32_t CMAR6;
    uint32_t RESERVED5;
    volatile uint32_t CCR7;
    volatile uint32_t CNDTR7;
    volatile uint32_t CPAR7;
    volatile uint32_t CMAR7;
} DMA_TypeDef;

#define RCC_BASE       ((uint32_t)0x40021000UL)
#define GPIOA_BASE     ((uint32_t)0x40010800UL)
#define GPIOB_BASE     ((uint32_t)0x40010C00UL)
#define SPI1_BASE      ((uint32_t)0x40013000UL)
#define SPI2_BASE      ((uint32_t)0x40003800UL)
#define DMA1_BASE      ((uint32_t)0x40020000UL)

#define RCC            ((RCC_TypeDef *) RCC_BASE)
#define GPIOA          ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB          ((GPIO_TypeDef *) GPIOB_BASE)
#define SPI1           ((SPI_TypeDef *) SPI1_BASE)
#define SPI2           ((SPI_TypeDef *) SPI2_BASE)
#define DMA1           ((DMA_TypeDef *) DMA1_BASE)

/* RCC enable bits */
#define RCC_APB2ENR_AFIOEN   (1U << 0)
#define RCC_APB2ENR_IOPAEN   (1U << 2)
#define RCC_APB2ENR_IOPBEN   (1U << 3)
#define RCC_APB2ENR_SPI1EN   (1U << 12)
#define RCC_APB1ENR_SPI2EN   (1U << 14)
#define RCC_AHBENR_DMA1EN    (1U << 0)

/* SPI_CR1 bits */
#define SPI_CR1_CPHA         (1U << 0)
#define SPI_CR1_CPOL         (1U << 1)
#define SPI_CR1_MSTR         (1U << 2)
#define SPI_CR1_BR_Pos       3U
#define SPI_CR1_SPE          (1U << 6)
#define SPI_CR1_LSBFIRST     (1U << 7)
#define SPI_CR1_SSI          (1U << 8)
#define SPI_CR1_SSM          (1U << 9)
#define SPI_CR1_RXONLY       (1U << 10)
#define SPI_CR1_DFF          (1U << 11)
#define SPI_CR1_BIDIOE       (1U << 14)
#define SPI_CR1_BIDIMODE     (1U << 15)

/* SPI_CR2 bits */
#define SPI_CR2_RXDMAEN      (1U << 0)
#define SPI_CR2_TXDMAEN      (1U << 1)

/* DMA CCR bits */
#define DMA_CCR_EN           (1U << 0)
#define DMA_CCR_TCIE         (1U << 1)
#define DMA_CCR_HTIE         (1U << 2)
#define DMA_CCR_TEIE         (1U << 3)
#define DMA_CCR_DIR          (1U << 4)
#define DMA_CCR_CIRC         (1U << 5)
#define DMA_CCR_PINC         (1U << 6)
#define DMA_CCR_MINC         (1U << 7)
#define DMA_CCR_PSIZE_Pos    8U
#define DMA_CCR_PSIZE_8BIT   (0U << 8)
#define DMA_CCR_PSIZE_16BIT  (1U << 8)
#define DMA_CCR_PSIZE_32BIT  (2U << 8)
#define DMA_CCR_MSIZE_Pos    10U
#define DMA_CCR_MSIZE_8BIT   (0U << 10)
#define DMA_CCR_MSIZE_16BIT  (1U << 10)
#define DMA_CCR_MSIZE_32BIT  (2U << 10)
#define DMA_CCR_PL_Pos       12U
#define DMA_CCR_PL_LOW       (0U << 12)
#define DMA_CCR_PL_MEDIUM    (1U << 12)
#define DMA_CCR_PL_HIGH      (2U << 12)
#define DMA_CCR_PL_VERYHIGH  (3U << 12)
#define DMA_CCR_MEM2MEM      (1U << 14)

/* DMA ISR/IFCR bits */
#define DMA_ISR_GIF1         (1U << 0)
#define DMA_ISR_TCIF1        (1U << 1)
#define DMA_ISR_HTIF1        (1U << 2)
#define DMA_ISR_TEIF1        (1U << 3)
#define DMA_ISR_GIF2         (1U << 4)
#define DMA_ISR_TCIF2        (1U << 5)
#define DMA_ISR_HTIF2        (1U << 6)
#define DMA_ISR_TEIF2        (1U << 7)
#define DMA_ISR_GIF3         (1U << 8)
#define DMA_ISR_TCIF3        (1U << 9)
#define DMA_ISR_HTIF3        (1U << 10)
#define DMA_ISR_TEIF3        (1U << 11)
#define DMA_ISR_GIF4         (1U << 12)
#define DMA_ISR_TCIF4        (1U << 13)
#define DMA_ISR_HTIF4        (1U << 14)
#define DMA_ISR_TEIF4        (1U << 15)
#define DMA_ISR_GIF5         (1U << 16)
#define DMA_ISR_TCIF5        (1U << 17)
#define DMA_ISR_HTIF5        (1U << 18)
#define DMA_ISR_TEIF5        (1U << 19)

/* SPI_SR bits */
#define SPI_SR_RXNE          (1U << 0)
#define SPI_SR_TXE           (1U << 1)
#define SPI_SR_BSY           (1U << 7)

#define GPIO_MODE_OUTPUT_50MHz 0x3U
#define GPIO_CNF_OUTPUT_AF_PP  0x2U
#define GPIO_CNF_INPUT_FLOAT   0x1U

static inline SPI_TypeDef *SPI_GetInstance(SPI_Instance_t instance)
{
    if (instance == SPI_INSTANCE_1) {
        return SPI1;
    }
    if (instance == SPI_INSTANCE_2) {
        return SPI2;
    }
    return NULL;
}

static inline void GPIO_ConfigPin(GPIO_TypeDef *port, uint8_t pin, uint32_t config)
{
    volatile uint32_t *reg = (pin < 8) ? &port->CRL : &port->CRH;
    uint8_t  shift = (pin % 8) * 4;
    uint32_t value = config << shift;
    *reg = (*reg & ~(0xFUL << shift)) | value;
}

static bool SPI_ConfigPins(SPI_Instance_t instance)
{
    if (instance == SPI_INSTANCE_1) {
        RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_SPI1EN;
        GPIO_ConfigPin(GPIOA, 5, (GPIO_MODE_OUTPUT_50MHz << 2) | GPIO_CNF_OUTPUT_AF_PP);
        GPIO_ConfigPin(GPIOA, 6, (0U << 2) | GPIO_CNF_INPUT_FLOAT);
        GPIO_ConfigPin(GPIOA, 7, (GPIO_MODE_OUTPUT_50MHz << 2) | GPIO_CNF_OUTPUT_AF_PP);
        return true;
    }

    if (instance == SPI_INSTANCE_2) {
        RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
        GPIO_ConfigPin(GPIOB, 13, (GPIO_MODE_OUTPUT_50MHz << 2) | GPIO_CNF_OUTPUT_AF_PP);
        GPIO_ConfigPin(GPIOB, 14, (0U << 2) | GPIO_CNF_INPUT_FLOAT);
        GPIO_ConfigPin(GPIOB, 15, (GPIO_MODE_OUTPUT_50MHz << 2) | GPIO_CNF_OUTPUT_AF_PP);
        return true;
    }

    return false;
}

static void SPI_WaitNotBusy(SPI_TypeDef *spi)
{
    while (spi->SR & SPI_SR_BSY) {
        __asm__("nop");
    }
}

bool SPI_InitMaster(SPI_Instance_t instance, uint32_t baud_prescaler, uint8_t mode)
{
    SPI_TypeDef *spi = SPI_GetInstance(instance);
    if (spi == NULL || mode > SPI_MODE_3) {
        return false;
    }

    if (!SPI_ConfigPins(instance)) {
        return false;
    }

    spi->CR1 &= ~SPI_CR1_SPE;
    SPI_WaitNotBusy(spi);

    uint32_t config = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | baud_prescaler;

    switch (mode) {
        case SPI_MODE_0:
            break;
        case SPI_MODE_1:
            config |= SPI_CR1_CPHA;
            break;
        case SPI_MODE_2:
            config |= SPI_CR1_CPOL;
            break;
        case SPI_MODE_3:
            config |= SPI_CR1_CPOL | SPI_CR1_CPHA;
            break;
    }

    spi->CR1 = (uint16_t)config;
    spi->CR2 = 0;
    spi->CR1 |= SPI_CR1_SPE;

    return true;
}

bool SPI_Enable(SPI_Instance_t instance, bool enable)
{
    SPI_TypeDef *spi = SPI_GetInstance(instance);
    if (spi == NULL) {
        return false;
    }

    if (enable) {
        spi->CR1 |= SPI_CR1_SPE;
    } else {
        spi->CR1 &= ~SPI_CR1_SPE;
        SPI_WaitNotBusy(spi);
    }

    return true;
}

uint8_t SPI_TransferByte(SPI_Instance_t instance, uint8_t data)
{
    SPI_TypeDef *spi = SPI_GetInstance(instance);
    if (spi == NULL) {
        return 0xFF;
    }

    while (!(spi->SR & SPI_SR_TXE)) {
        __asm__("nop");
    }

    spi->DR = data;

    while (!(spi->SR & SPI_SR_RXNE)) {
        __asm__("nop");
    }

    return (uint8_t)(spi->DR & 0xFFU);
}

bool SPI_IsBusy(SPI_Instance_t instance)
{
    SPI_TypeDef *spi = SPI_GetInstance(instance);
    return (spi != NULL) ? ((spi->SR & SPI_SR_BSY) != 0U) : false;
}

/* DMA Functions */

static inline volatile uint32_t *DMA_GetCCR(DMA_TypeDef *dma, uint8_t channel)
{
    switch (channel) {
        case 1: return &dma->CCR1;
        case 2: return &dma->CCR2;
        case 3: return &dma->CCR3;
        case 4: return &dma->CCR4;
        case 5: return &dma->CCR5;
        case 6: return &dma->CCR6;
        case 7: return &dma->CCR7;
        default: return NULL;
    }
}

static inline volatile uint32_t *DMA_GetCNDTR(DMA_TypeDef *dma, uint8_t channel)
{
    switch (channel) {
        case 1: return &dma->CNDTR1;
        case 2: return &dma->CNDTR2;
        case 3: return &dma->CNDTR3;
        case 4: return &dma->CNDTR4;
        case 5: return &dma->CNDTR5;
        case 6: return &dma->CNDTR6;
        case 7: return &dma->CNDTR7;
        default: return NULL;
    }
}

static inline volatile uint32_t *DMA_GetCPAR(DMA_TypeDef *dma, uint8_t channel)
{
    switch (channel) {
        case 1: return &dma->CPAR1;
        case 2: return &dma->CPAR2;
        case 3: return &dma->CPAR3;
        case 4: return &dma->CPAR4;
        case 5: return &dma->CPAR5;
        case 6: return &dma->CPAR6;
        case 7: return &dma->CPAR7;
        default: return NULL;
    }
}

static inline volatile uint32_t *DMA_GetCMAR(DMA_TypeDef *dma, uint8_t channel)
{
    switch (channel) {
        case 1: return &dma->CMAR1;
        case 2: return &dma->CMAR2;
        case 3: return &dma->CMAR3;
        case 4: return &dma->CMAR4;
        case 5: return &dma->CMAR5;
        case 6: return &dma->CMAR6;
        case 7: return &dma->CMAR7;
        default: return NULL;
    }
}

static void DMA_GetChannels(SPI_Instance_t instance, uint8_t *tx_channel, uint8_t *rx_channel)
{
    if (instance == SPI_INSTANCE_1) {
        *tx_channel = 3;  // SPI1_TX: DMA1 Channel 3
        *rx_channel = 2;  // SPI1_RX: DMA1 Channel 2
    } else if (instance == SPI_INSTANCE_2) {
        *tx_channel = 5;  // SPI2_TX: DMA1 Channel 5
        *rx_channel = 4;  // SPI2_RX: DMA1 Channel 4
    }
}

bool SPI_EnableDMA(SPI_Instance_t instance, bool tx_enable, bool rx_enable)
{
    SPI_TypeDef *spi = SPI_GetInstance(instance);
    if (spi == NULL) {
        return false;
    }

    // Enable DMA1 clock
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    uint16_t cr2 = spi->CR2 & ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

    if (tx_enable) {
        cr2 |= SPI_CR2_TXDMAEN;
    }
    if (rx_enable) {
        cr2 |= SPI_CR2_RXDMAEN;
    }

    spi->CR2 = cr2;
    return true;
}

bool SPI_TransmitDMA(SPI_Instance_t instance, const uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0) {
        return false;
    }

    uint8_t tx_channel, rx_channel;
    DMA_GetChannels(instance, &tx_channel, &rx_channel);

    SPI_TypeDef *spi = SPI_GetInstance(instance);
    if (spi == NULL) {
        return false;
    }

    // Disable DMA channel
    *DMA_GetCCR(DMA1, tx_channel) &= ~DMA_CCR_EN;

    // Clear DMA flags
    DMA1->IFCR = (DMA_ISR_TCIF1 << ((tx_channel - 1) * 4)) |
                 (DMA_ISR_HTIF1 << ((tx_channel - 1) * 4)) |
                 (DMA_ISR_TEIF1 << ((tx_channel - 1) * 4)) |
                 (DMA_ISR_GIF1 << ((tx_channel - 1) * 4));

    // Configure DMA
    *DMA_GetCNDTR(DMA1, tx_channel) = size;
    *DMA_GetCPAR(DMA1, tx_channel) = (uint32_t)&spi->DR;
    *DMA_GetCMAR(DMA1, tx_channel) = (uint32_t)data;

    // Configure CCR: Memory to Peripheral, 8-bit, increment memory, enable
    *DMA_GetCCR(DMA1, tx_channel) = DMA_CCR_DIR | DMA_CCR_MINC |
                                    DMA_CCR_PSIZE_8BIT | DMA_CCR_MSIZE_8BIT |
                                    DMA_CCR_PL_HIGH | DMA_CCR_TCIE;

    // Enable DMA channel
    *DMA_GetCCR(DMA1, tx_channel) |= DMA_CCR_EN;

    return true;
}

bool SPI_ReceiveDMA(SPI_Instance_t instance, uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0) {
        return false;
    }

    uint8_t tx_channel, rx_channel;
    DMA_GetChannels(instance, &tx_channel, &rx_channel);

    SPI_TypeDef *spi = SPI_GetInstance(instance);
    if (spi == NULL) {
        return false;
    }

    // Disable DMA channel
    *DMA_GetCCR(DMA1, rx_channel) &= ~DMA_CCR_EN;

    // Clear DMA flags
    DMA1->IFCR = (DMA_ISR_TCIF1 << ((rx_channel - 1) * 4)) |
                 (DMA_ISR_HTIF1 << ((rx_channel - 1) * 4)) |
                 (DMA_ISR_TEIF1 << ((rx_channel - 1) * 4)) |
                 (DMA_ISR_GIF1 << ((rx_channel - 1) * 4));

    // Configure DMA
    *DMA_GetCNDTR(DMA1, rx_channel) = size;
    *DMA_GetCPAR(DMA1, rx_channel) = (uint32_t)&spi->DR;
    *DMA_GetCMAR(DMA1, rx_channel) = (uint32_t)data;

    // Configure CCR: Peripheral to Memory, 8-bit, increment memory, enable
    *DMA_GetCCR(DMA1, rx_channel) = DMA_CCR_MINC |
                                    DMA_CCR_PSIZE_8BIT | DMA_CCR_MSIZE_8BIT |
                                    DMA_CCR_PL_HIGH | DMA_CCR_TCIE;

    // Enable DMA channel
    *DMA_GetCCR(DMA1, rx_channel) |= DMA_CCR_EN;

    return true;
}

bool SPI_IsDMATxComplete(SPI_Instance_t instance)
{
    uint8_t tx_channel, rx_channel;
    DMA_GetChannels(instance, &tx_channel, &rx_channel);

    uint32_t flag = DMA_ISR_TCIF1 << ((tx_channel - 1) * 4);
    return (DMA1->ISR & flag) != 0;
}

bool SPI_IsDMARxComplete(SPI_Instance_t instance)
{
    uint8_t tx_channel, rx_channel;
    DMA_GetChannels(instance, &tx_channel, &rx_channel);

    uint32_t flag = DMA_ISR_TCIF1 << ((rx_channel - 1) * 4);
    return (DMA1->ISR & flag) != 0;
}
