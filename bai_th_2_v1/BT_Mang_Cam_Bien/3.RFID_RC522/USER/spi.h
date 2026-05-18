/**
 * @file    spi.h
 * @brief   Thu vien SPI1 cho STM32F10x (Standard Peripheral Library)
 *          Ho tro truyen/nhan 1 byte, ghi/doc thanh ghi thiet bi slave
 *
 * Phan cung su dung:
 *   - SPI1: PA5 (SCK), PA6 (MISO), PA7 (MOSI), PA4 (CS - dieu khien bang phan mem)
 *   - Toc do: fPCLK/8 = 9MHz (SYSCLK = 72MHz)
 *   - Che do: SPI Mode 0 (CPOL=0, CPHA=0), MSB First, 8-bit
 */

#ifndef SPI_H
#define SPI_H

#include "define.h"

//===========================//
//      CHAN CS (NSS)        //
//===========================//

/** Chan CS (Chip Select) - dieu khien bang phan mem */
#define SPI1_CS_PORT    GPIOA
#define SPI1_CS_PIN     GPIO_Pin_4

//===========================//
//        MACRO CS           //
//===========================//

#define SPI1_CS_Low()   GPIO_ResetBits(SPI1_CS_PORT, SPI1_CS_PIN)
#define SPI1_CS_High()  GPIO_SetBits(SPI1_CS_PORT, SPI1_CS_PIN)

//===========================//
//        API CONG KHAI      //
//===========================//

/**
 * @brief  Khoi dong ngoai vi SPI1 va cac chan GPIO tuong ung
 *         PA4 -> CS (Output PP)
 *         PA5 -> SCK (AF PP)
 *         PA6 -> MISO (Input Floating)
 *         PA7 -> MOSI (AF PP)
 *         Toc do SPI: fPCLK/8, Mode 0, MSB First, 8-bit
 */
void SPI1_Config(void);

/**
 * @brief  Truyen va nhan dong thoi 1 byte qua SPI (Full-Duplex)
 * @param  data  Byte can truyen di
 * @return Byte nhan duoc tu slave trong cung ky truyen
 */
uint8_t SPI1_TransmitReceive(uint8_t data);

/**
 * @brief  Ghi 1 byte du lieu vao thanh ghi cua thiet bi slave qua SPI
 *         Quy trinh: CS_Low -> gui addr (write) -> gui data -> CS_High
 * @param  reg   Dia chi thanh ghi (7-bit, bit7=0 la write)
 * @param  data  Byte du lieu can ghi
 */
void SPI1_WriteReg(uint8_t reg, uint8_t data);

/**
 * @brief  Doc 1 byte du lieu tu thanh ghi cua thiet bi slave qua SPI
 *         Quy trinh: CS_Low -> gui addr (read) -> nhan data -> CS_High
 * @param  reg  Dia chi thanh ghi (7-bit, bit7=1 la read)
 * @return Byte du lieu doc duoc tu slave
 */
uint8_t SPI1_ReadReg(uint8_t reg);

#endif /* SPI_H */
