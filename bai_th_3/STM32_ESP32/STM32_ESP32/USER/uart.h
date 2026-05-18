#ifndef __UART_H__
#define __UART_H__

#include "stm32f10x.h"
#include <string.h>

#define MAX_BUFFER  128
#define START_BYTE  0xAA
#define END_BYTE    0x55
#define CMD_TEXT    0x01
#define CMD_ACK     0x02

typedef struct {
    uint8_t start;
    uint8_t cmd;
    uint8_t len;
    uint8_t data[64];
    uint8_t checksum;
    uint8_t end;
} Message_t;

// Khoi tao
void GPIO_Config_TX_RX(void);
void USART1_Config(void);
void USART2_Config(void);

// Giao tiep PC (USART1)
void    PC_Print(const char *str);
void    PC_Println(const char *str);
uint8_t PC_ReadLine(char *buf, uint8_t maxlen);

// Gui so nguyen (debug)
void USART1_Send_Number(int16_t num);

// Giao thuc ban tin voi ESP32 (USART2)
uint8_t MSG_Checksum(Message_t *msg);
void    MSG_Build(Message_t *msg, uint8_t cmd, uint8_t *data, uint8_t len);
void    MSG_Send_ESP32(Message_t *msg);
uint8_t MSG_Receive_ESP32(Message_t *out_msg);

// IRQ handlers (khai bao de linker khong canh bao)
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);

#endif