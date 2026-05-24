#ifndef UART_H
#define UART_H

#include "main.h"
#include <stdint.h>

void UART1_Init(void);
void UART1_SendChar(char c);
void UART1_SendString(char *str);
void UART1_SendHexByte(uint8_t data);
void UART1_SendUID(uint8_t *uid);

#endif