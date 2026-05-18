#ifndef __UART__
#define __UART__

#include "define.h"

void Uart_Init(void);
void Uart_Gpio_TxRx_Init(void);
void Uart_SendChar(char _chr);
void Uart_SendStr(char *str);
void Uart_SendHex(uint8_t val);
void Uart_SendInt(int number);
void Uart_SendFloat(float number);
uint16_t Uart_ReadChar(void);
uint8_t Uart_Compare(char *str1, char * str2);
void USART1_IRQHandler(void);
uint8_t Uart_Debug_Flag(void);

#endif
