#ifndef __DHT11__
#define __DHT11__

#include "define.h"

// Khoi tao cam bien DHT11 (thiet lap GPIO, che do ban dau)
void DHT11_Init(void);

// Doc du lieu tu cam bien DHT11
void DHT11_Read(void);

// Lay gia tri nhiet do tu DHT11 (don vi: do C)
float DHT11_Get_Temperature(void);

// Lay gia tri do am tu DHT11 (don vi: %)
float DHT11_Get_Humidity(void);

#endif
