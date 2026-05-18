#include "uart.h"

void Uart_Gpio_TxRx_Init(void)
{
    GPIO_InitTypeDef gpio_typedef;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // TX - PA9
    gpio_typedef.GPIO_Pin = GPIO_Pin_9;
    gpio_typedef.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_typedef.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_typedef);

    // RX - PA10
    gpio_typedef.GPIO_Pin = GPIO_Pin_10;
    gpio_typedef.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_typedef);
}

void Uart_Init(void)
{
    USART_InitTypeDef usart_typedef;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    usart_typedef.USART_BaudRate = 9600;
    usart_typedef.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_typedef.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usart_typedef.USART_Parity = USART_Parity_No;
    usart_typedef.USART_StopBits = USART_StopBits_1;
    usart_typedef.USART_WordLength = USART_WordLength_8b;
    
    USART_Init(USART1, &usart_typedef);
    USART_Cmd(USART1, ENABLE);
}

void Uart_SendChar(char _chr)
{
    USART_SendData(USART1, _chr);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void Uart_SendStr(char *str)
{
    while(*str) {
        Uart_SendChar(*str++);
    }
}

void Uart_SendInt(int number)
{
    char digit[10] = "";
    int count = 0;
    
    if (number == 0) {
        Uart_SendChar('0');
        return;
    }
    if (number < 0) {
        Uart_SendChar('-');
        number = -number;
    }
    while (number != 0) {
        digit[count++] = number % 10;
        number = number / 10;
    }
    while (count != 0) {
        Uart_SendChar(digit[count - 1] + 0x30);
        count--;
    }
}
void Uart_SendFloat(float number)
{
    char buffer[20];
    // In s? th?c v?i 2 ch? s? th?p phân
    sprintf(buffer, "%.2f", number ); 
    Uart_SendStr(buffer);
}
