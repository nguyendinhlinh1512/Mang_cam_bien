/*
 * dht22.h
 * Giao tiep cam bien DHT22 tren STM32F103
 * Data pin: PA1
 */

#ifndef __DHT22_H__
#define __DHT22_H__

#include "stm32f10x.h"
#include "tim2.h"

typedef struct {
    float temperature; // do C
    float humidity;    // %
    uint8_t valid;     // 1 = doc thanh cong
} DHT22_Data_t;

void DHT22_Init(void);
DHT22_Data_t DHT22_Read(void);

#endif