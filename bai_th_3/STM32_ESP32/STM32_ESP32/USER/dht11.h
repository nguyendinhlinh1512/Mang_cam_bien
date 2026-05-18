#ifndef __DHT11__
#define __DHT11__

#include "stm32f10x.h"
#include "tim2.h"


#ifdef __cplusplus
extern "C" {
#endif

extern int16_t g_temp; // da tao o file .c k can khai bao lai
extern int16_t g_hum;

void DHT11_Init(void);
void DHT11_Read(void);

#ifdef __cplusplus
}
#endif

#endif