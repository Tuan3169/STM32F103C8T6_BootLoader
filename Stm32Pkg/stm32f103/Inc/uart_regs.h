/*
 * Minimal UART (USART1/2/3) register-level driver for STM32F103.
 */

#ifndef STM32F103_UART_REGS_H
#define STM32F103_UART_REGS_H

#include <stdint.h>
#include <stdbool.h>

#define UART_DATA_SIZE 64U

typedef enum {
	UART_PORT1 = 0,
	UART_PORT2 = 1,
	UART_PORT3 = 2
} UartPort;

typedef void (*UartRxCallback)(uint8_t byte);

void UART_Init(UartPort port, uint32_t baudrate);
void UART_SendBytes(UartPort port, const uint8_t *data, uint32_t length);
void UART_SendString(UartPort port, const char *text);
bool UART_ReadByte(UartPort port, uint8_t *out_byte);
uint32_t UART_Receive(UartPort port, uint8_t *buffer, uint32_t max_length);
void UART_EnableRxInterrupt(UartPort port, UartRxCallback callback);
void UART_DisableRxInterrupt(UartPort port);

#endif /* STM32F103_UART_REGS_H */
