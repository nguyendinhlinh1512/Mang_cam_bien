/**
 * @file    i2c.h
 * @brief   Thu vien I2C1 cho STM32F10x (Standard Peripheral Library)
 *          Ho tro ghi 1 byte, doc 1 byte, doc nhieu byte lien tiep
 *
 * Phan cung su dung:
 *   - I2C1: PB6 (SCL), PB7 (SDA)
 *   - Toc do: 100kHz (Standard Mode)
 */

#ifndef I2C_H
#define I2C_H

#include "define.h"

//===========================//
//        API CONG KHAI      //
//===========================//

/**
 * @brief  Khoi dong ngoai vi I2C1 va cac chan GPIO tuong ung
 *         PB6 -> SCL, PB7 -> SDA, che do AF Open-Drain, 100kHz
 */
void I2C1_Config(void);

/**
 * @brief  Ghi 1 byte du lieu vao thanh ghi cua thiet bi slave
 * @param  addr  Dia chi I2C cua slave (da dich trai 1 bit, vi du: 0x76 << 1)
 * @param  reg   Dia chi thanh ghi ben trong slave
 * @param  data  Byte du lieu can ghi
 */
void I2C1_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);

/**
 * @brief  Doc 1 byte du lieu tu thanh ghi cua thiet bi slave
 * @param  addr  Dia chi I2C cua slave (da dich trai 1 bit)
 * @param  reg   Dia chi thanh ghi can doc
 * @return Gia tri byte doc duoc
 */
uint8_t I2C1_ReadReg(uint8_t addr, uint8_t reg);

/**
 * @brief  Doc nhieu byte lien tiep tu thiet bi slave (auto-increment)
 * @param  addr  Dia chi I2C cua slave (da dich trai 1 bit)
 * @param  reg   Dia chi thanh ghi bat dau
 * @param  data  Con tro toi vung bo nho luu du lieu doc duoc
 * @param  len   So byte can doc
 */
void I2C1_ReadMulti(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);

#endif 
