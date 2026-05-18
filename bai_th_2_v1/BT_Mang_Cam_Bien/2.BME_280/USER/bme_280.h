/**
 *    bme280.h
 *   Thu vien BMP280 / BME280 su dung giao thuc I2C
 *          Tuong thich voi ca hai cam bien BMP280 (ID=0x58) va BME280 (ID=0x60)
 *
 * Phu thuoc: i2c.h / i2c.c
 */

#ifndef BME280_H
#define BME280_H

#include "define.h"

//===========================//
//      DIA CHI & CHIP ID    //
//===========================//

/** Dia chi I2C mac dinh khi SDO = GND (da dich trai 1 bit cho STM32 SPL) */
#define BME280_I2C_ADDR_DEFAULT   (0x76 << 1)

/** Dia chi I2C thu hai khi SDO = VCC */
#define BME280_I2C_ADDR_SECONDARY (0x77 << 1)

/** Chip ID mong doi */
#define BMP280_CHIP_ID  0x58
#define BME280_CHIP_ID  0x60

//===========================//
//   CAU TRUC HE SO HIEU CHUAN
//===========================//

/**
 *Chua toan bo he so hieu chuan nhiet do va ap suat doc tu ROM cam bien
 */
typedef struct {
    /* He so nhiet do */
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    /* He so ap suat */
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BME280_CalibData;

//===========================//
//       CAU TRUC KET QUA    //
//===========================//

/**
 *  Ket qua do sau khi da bu tru hieu chuan
 */
typedef struct {
    float temperature; /**< Nhiet do (don vi: do C)  */
    float pressure;    /**< Ap suat   (don vi: hPa)  */
} BME280_Data;

//===========================//
//          API              //
//===========================//

/**
 *   Khoi dong cam bien: kiem tra Chip ID, cau hinh che do do,
 *         va doc toan bo he so hieu chuan
 *   i2c_addr  Dia chi I2C (vi du: BME280_I2C_ADDR_DEFAULT)
 * 1 neu thanh cong, 0 neu Chip ID sai
 */
uint8_t BME280_Init(uint8_t i2c_addr);

/**
 *  Doc va tinh toan nhiet do tu ADC raw + he so hieu chuan
 *  i2c_addr  Dia chi I2C cua cam bien
 *  Nhiet do (do C), do chinh xac 0.01 C
 */
float BME280_ReadTemperature(uint8_t i2c_addr);

/**
 *  Doc va tinh toan ap suat tu ADC raw + he so hieu chuan
 *         Luu y: phai goi BME280_ReadTemperature() truoc de cap nhat t_fine
 * i2c_addr  Dia chi I2C cua cam bien
 * Ap suat (hPa), do chinh xac 0.01 hPa
 */
float BME280_ReadPressure(uint8_t i2c_addr);

/**
 *  Doc ca nhiet do va ap suat trong mot lan goi (hieu qua hon)
 *  i2c_addr  Dia chi I2C cua cam bien
 *  out       Con tro toi cau truc BME280_Data nhan ket qua
 */
void BME280_ReadAll(uint8_t i2c_addr, BME280_Data *out);

#endif 