#ifndef __UART_H
#define __UART_H

#include "define.h"

void Uart_Init(void);
void Uart_Gpio_TxRx_Init(void);
void Uart_SendChar(char _chr);
void Uart_SendStr(char *str);
void Uart_SendInt(int number);
// Thêm hàm g?i s? th?c
void Uart_SendFloat(float number); 

#endif
