#include "../Inc/uart_regs.h"

// Base addresses
#define RCC_BASE        0x40021000U
#define GPIOA_BASE      0x40010800U
#define GPIOB_BASE      0x40010C00U
#define USART1_BASE     0x40013800U
#define USART2_BASE     0x40004400U
#define USART3_BASE     0x40004800U

// RCC registers
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18U))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1CU))

// GPIOA registers
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04U))
#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00U))

// GPIOB registers
#define GPIOB_CRH       (*(volatile uint32_t *)(GPIOB_BASE + 0x04U))

#define USART_SR(base)  (*(volatile uint32_t *)((base) + 0x00U))
#define USART_DR(base)  (*(volatile uint32_t *)((base) + 0x04U))
#define USART_BRR(base) (*(volatile uint32_t *)((base) + 0x08U))
#define USART_CR1(base) (*(volatile uint32_t *)((base) + 0x0CU))
#define USART_CR2(base) (*(volatile uint32_t *)((base) + 0x10U))
#define USART_CR3(base) (*(volatile uint32_t *)((base) + 0x14U))

// RCC APB2 enable bits
#define RCC_APB2ENR_AFIOEN   (1U << 0U)
#define RCC_APB2ENR_IOPAEN   (1U << 2U)
#define RCC_APB2ENR_IOPBEN   (1U << 3U)
#define RCC_APB2ENR_USART1EN (1U << 14U)

// RCC APB1 enable bits
#define RCC_APB1ENR_USART2EN (1U << 17U)
#define RCC_APB1ENR_USART3EN (1U << 18U)

// USART status bits
#define USART_SR_TXE     (1U << 7U)
#define USART_SR_RXNE    (1U << 5U)

// USART control bits
#define USART_CR1_UE     (1U << 13U)
#define USART_CR1_TE     (1U << 3U)
#define USART_CR1_RE     (1U << 2U)
#define USART_CR1_RXNEIE (1U << 5U)

// NVIC registers
#define NVIC_ISER0       (*(volatile uint32_t *)0xE000E100U)
#define NVIC_ISER1       (*(volatile uint32_t *)0xE000E104U)
#define NVIC_ICER0       (*(volatile uint32_t *)0xE000E180U)
#define NVIC_ICER1       (*(volatile uint32_t *)0xE000E184U)

// IRQ numbers (STM32F103)
#define IRQ_USART1       37U
#define IRQ_USART2       38U
#define IRQ_USART3       39U

// GPIO config shifts
#define GPIO_CR_MASK        0xFU
#define GPIO_CR_PA2_SHIFT   8U
#define GPIO_CR_PA3_SHIFT   12U
#define GPIO_CR_PA9_SHIFT   4U
#define GPIO_CR_PA10_SHIFT  8U
#define GPIO_CR_PB10_SHIFT  8U
#define GPIO_CR_PB11_SHIFT  12U

#define UART_PCLK2_HZ 72000000U
#define UART_PCLK1_HZ 36000000U

static UartRxCallback g_uart_rx_callbacks[3] = {0};

static uint32_t UART_ComputeBrr(uint32_t pclk_hz, uint32_t baudrate)
{
	// USARTDIV = pclk / (16 * baud)
	// BRR = mantissa << 4 | fraction
	uint32_t divisor = (pclk_hz + (baudrate / 2U)) / baudrate;
	uint32_t mantissa = divisor / 16U;
	uint32_t fraction = divisor - (mantissa * 16U);
	return (mantissa << 4U) | (fraction & 0xFU);
}

static uint32_t UART_GetBase(UartPort port)
{
	switch (port) {
		case UART_PORT1:
			return USART1_BASE;
		case UART_PORT2:
			return USART2_BASE;
		case UART_PORT3:
			return USART3_BASE;
		default:
			return USART1_BASE;
	}
}

static void UART_EnableNvicIrq(UartPort port)
{
	uint32_t irqn = IRQ_USART1;
	if (port == UART_PORT2) {
		irqn = IRQ_USART2;
	} else if (port == UART_PORT3) {
		irqn = IRQ_USART3;
	}

	if (irqn < 32U) {
		NVIC_ISER0 = (1U << irqn);
	} else {
		NVIC_ISER1 = (1U << (irqn - 32U));
	}
}

static void UART_DisableNvicIrq(UartPort port)
{
	uint32_t irqn = IRQ_USART1;
	if (port == UART_PORT2) {
		irqn = IRQ_USART2;
	} else if (port == UART_PORT3) {
		irqn = IRQ_USART3;
	}

	if (irqn < 32U) {
		NVIC_ICER0 = (1U << irqn);
	} else {
		NVIC_ICER1 = (1U << (irqn - 32U));
	}
}

static void UART_IrqHandler(UartPort port)
{
	uint32_t base = UART_GetBase(port);
	uint32_t sr = USART_SR(base);

	if ((sr & USART_SR_RXNE) != 0U) {
		uint8_t value = (uint8_t)(USART_DR(base) & 0xFFU);
		UartRxCallback cb = g_uart_rx_callbacks[port];
		if (cb != 0U) {
			cb(value);
		}
	}
}

void UART_Init(UartPort port, uint32_t baudrate)
{
	uint32_t base = UART_GetBase(port);
	uint32_t pclk = UART_PCLK2_HZ;

	// Enable clocks: AFIO, GPIOx, USARTx
	RCC_APB2ENR |= RCC_APB2ENR_AFIOEN;

	if (port == UART_PORT1) {
		RCC_APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN);
		uint32_t crh = GPIOA_CRH;
		crh &= ~((GPIO_CR_MASK << GPIO_CR_PA9_SHIFT) | (GPIO_CR_MASK << GPIO_CR_PA10_SHIFT));
		crh |= (0xBU << GPIO_CR_PA9_SHIFT);  // PA9: AF push-pull 50MHz
		crh |= (0x4U << GPIO_CR_PA10_SHIFT); // PA10: input floating
		GPIOA_CRH = crh;
		pclk = UART_PCLK2_HZ;
	} else if (port == UART_PORT2) {
		RCC_APB2ENR |= RCC_APB2ENR_IOPAEN;
		RCC_APB1ENR |= RCC_APB1ENR_USART2EN;
		uint32_t crl = GPIOA_CRL;
		crl &= ~((GPIO_CR_MASK << GPIO_CR_PA2_SHIFT) | (GPIO_CR_MASK << GPIO_CR_PA3_SHIFT));
		crl |= (0xBU << GPIO_CR_PA2_SHIFT);  // PA2: AF push-pull 50MHz
		crl |= (0x4U << GPIO_CR_PA3_SHIFT);  // PA3: input floating
		GPIOA_CRL = crl;
		pclk = UART_PCLK1_HZ;
	} else {
		RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
		RCC_APB1ENR |= RCC_APB1ENR_USART3EN;
		uint32_t crh = GPIOB_CRH;
		crh &= ~((GPIO_CR_MASK << GPIO_CR_PB10_SHIFT) | (GPIO_CR_MASK << GPIO_CR_PB11_SHIFT));
		crh |= (0xBU << GPIO_CR_PB10_SHIFT); // PB10: AF push-pull 50MHz
		crh |= (0x4U << GPIO_CR_PB11_SHIFT); // PB11: input floating
		GPIOB_CRH = crh;
		pclk = UART_PCLK1_HZ;
	}

	// Disable USART before configuration
	USART_CR1(base) = 0U;
	USART_CR2(base) = 0U;
	USART_CR3(base) = 0U;

	USART_BRR(base) = UART_ComputeBrr(pclk, baudrate);

	// Enable USART, TX, RX
	USART_CR1(base) = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void UART_SendBytes(UartPort port, const uint8_t *data, uint32_t length)
{
	uint32_t base = UART_GetBase(port);

	if (data == 0U || length == 0U) {
		return;
	}

	for (uint32_t i = 0U; i < length; ++i) {
		while ((USART_SR(base) & USART_SR_TXE) == 0U) {
			// wait for TX buffer empty
		}
		USART_DR(base) = data[i];
	}
}

void UART_SendString(UartPort port, const char *text)
{
	uint32_t base = UART_GetBase(port);

	if (text == 0U) {
		return;
	}

	while (*text != '\0') {
		while ((USART_SR(base) & USART_SR_TXE) == 0U) {
			// wait for TX buffer empty
		}
		USART_DR(base) = (uint8_t)(*text);
		++text;
	}
}

bool UART_ReadByte(UartPort port, uint8_t *out_byte)
{
	uint32_t base = UART_GetBase(port);

	if ((USART_SR(base) & USART_SR_RXNE) == 0U) {
		return false;
	}

	uint8_t value = (uint8_t)(USART_DR(base) & 0xFFU);
	if (out_byte != 0U) {
		*out_byte = value;
	}
	return true;
}

uint32_t UART_Receive(UartPort port, uint8_t *buffer, uint32_t max_length)
{
	if (buffer == 0U || max_length == 0U) {
		return 0U;
	}

	uint32_t count = 0U;
	while ((count < max_length) && UART_ReadByte(port, &buffer[count])) {
		++count;
	}
	return count;
}

void UART_EnableRxInterrupt(UartPort port, UartRxCallback callback)
{
	uint32_t base = UART_GetBase(port);
	g_uart_rx_callbacks[port] = callback;
	USART_CR1(base) |= USART_CR1_RXNEIE;
	UART_EnableNvicIrq(port);
}

void UART_DisableRxInterrupt(UartPort port)
{
	uint32_t base = UART_GetBase(port);
	USART_CR1(base) &= ~USART_CR1_RXNEIE;
	UART_DisableNvicIrq(port);
	g_uart_rx_callbacks[port] = 0U;
}

void USART1_IRQHandler(void)
{
	UART_IrqHandler(UART_PORT1);
}

void USART2_IRQHandler(void)
{
	UART_IrqHandler(UART_PORT2);
}

void USART3_IRQHandler(void)
{
	UART_IrqHandler(UART_PORT3);
}
