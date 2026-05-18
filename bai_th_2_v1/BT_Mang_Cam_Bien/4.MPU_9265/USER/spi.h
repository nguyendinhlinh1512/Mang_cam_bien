#ifndef SPI_H
#define SPI_H

#include "define.h"

#define SPI1_CS_PORT    GPIOA
#define SPI1_CS_PIN     GPIO_Pin_4

#define SPI1_CS_Low()   GPIO_ResetBits(SPI1_CS_PORT, SPI1_CS_PIN)
#define SPI1_CS_High()  GPIO_SetBits(SPI1_CS_PORT, SPI1_CS_PIN)

void SPI1_Config(void);
uint8_t SPI1_TransmitReceive(uint8_t data);

#endif
