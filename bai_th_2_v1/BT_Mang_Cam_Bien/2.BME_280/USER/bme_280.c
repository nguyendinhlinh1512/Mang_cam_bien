#include "bme_280.h"

//===========================//
//   DIA CHI THANH GHI NB   //
//===========================//

#define REG_CHIP_ID     0xD0  
#define REG_RESET       0xE0  /*  (ghi 0xB6 de reset)     */
#define REG_CTRL_MEAS   0xF4  
#define REG_CONFIG      0xF5  /* Cai dat standby time, filter, SPI       */
#define REG_PRESS_MSB   0xF7  
#define REG_TEMP_MSB    0xFA  
#define REG_CALIB_START 0x88  
#define REG_CALIB_LEN   24    


static BME280_CalibData s_calib;    /* He so hieu chuan luu noi bo         */
static int32_t          s_t_fine;   /* Bien trung gian nhiet do -> ap suat */



/**
 *  Doc 24 byte he so hieu chuan tu ROM cam bien va phan tich
 */
static void _ReadCalibration(uint8_t i2c_addr)
{
    uint8_t calib[REG_CALIB_LEN];
    I2C1_ReadMulti(i2c_addr, REG_CALIB_START, calib, REG_CALIB_LEN);

    s_calib.dig_T1 = (uint16_t)(calib[1]  << 8 | calib[0]);
    s_calib.dig_T2 = (int16_t) (calib[3]  << 8 | calib[2]);
    s_calib.dig_T3 = (int16_t) (calib[5]  << 8 | calib[4]);

    s_calib.dig_P1 = (uint16_t)(calib[7]  << 8 | calib[6]);
    s_calib.dig_P2 = (int16_t) (calib[9]  << 8 | calib[8]);
    s_calib.dig_P3 = (int16_t) (calib[11] << 8 | calib[10]);
    s_calib.dig_P4 = (int16_t) (calib[13] << 8 | calib[12]);
    s_calib.dig_P5 = (int16_t) (calib[15] << 8 | calib[14]);
    s_calib.dig_P6 = (int16_t) (calib[17] << 8 | calib[16]);
    s_calib.dig_P7 = (int16_t) (calib[19] << 8 | calib[18]);
    s_calib.dig_P8 = (int16_t) (calib[21] << 8 | calib[20]);
    s_calib.dig_P9 = (int16_t) (calib[23] << 8 | calib[22]);
}


//     BME280_Init       

uint8_t BME280_Init(uint8_t i2c_addr)
{
    uint8_t id = I2C1_ReadReg(i2c_addr, REG_CHIP_ID);

    if (id != BMP280_CHIP_ID && id != BME280_CHIP_ID)
        return 0; /* Chip ID khong hop le */

    /* Cau hinh che do do:
     *   osrs_t=x1, osrs_p=x1, Normal mode */
    I2C1_WriteReg(i2c_addr, REG_CTRL_MEAS, 0x27);

    /* Cau hinh standby time = 1000ms, filter off */
    I2C1_WriteReg(i2c_addr, REG_CONFIG,    0xA0);

    /* Doc he so hieu chuan */
    _ReadCalibration(i2c_addr);

    return 1; /* Khoi dong thanh cong */
}


//   BME280_ReadTemperature 

float BME280_ReadTemperature(uint8_t i2c_addr)
{
    uint8_t  raw[3];
    int32_t  adc_T, var1, var2;
    float    T;

    /* Doc 3 byte nhiet do: temp_msb, temp_lsb, temp_xlsb */
    I2C1_ReadMulti(i2c_addr, REG_TEMP_MSB, raw, 3);

    /* Ghep thanh gia tri 20-bit */
    adc_T = ((uint32_t)raw[0] << 12) |
            ((uint32_t)raw[1] <<  4) |
            (raw[2] >> 4);

    /* Cong thuc bu tru nhiet do (datasheet Bosch BMP280, muc 4.2.3) */
    var1 = ((((adc_T >> 3) - ((int32_t)s_calib.dig_T1 << 1))) *
             ((int32_t)s_calib.dig_T2)) >> 11;

    var2 = (((((adc_T >> 4) - ((int32_t)s_calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)s_calib.dig_T1))) >> 12) *
             ((int32_t)s_calib.dig_T3)) >> 14;

    /* t_fine dung chung cho tinh ap suat */
    s_t_fine = var1 + var2;

    /* Ket qua do C, do chinh xac 0.01 C */
    T = (float)((s_t_fine * 5 + 128) >> 8) / 100.0f;
    return T;
}

//    BME280_ReadPressure   

float BME280_ReadPressure(uint8_t i2c_addr)
{
    uint8_t raw[3];
    int32_t adc_P;
    int64_t var1, var2, p;
    float   press;

    /* Doc 3 byte ap suat: press_msb, press_lsb, press_xlsb */
    I2C1_ReadMulti(i2c_addr, REG_PRESS_MSB, raw, 3);

    /* Ghep thanh gia tri 20-bit */
    adc_P = ((uint32_t)raw[0] << 12) |
            ((uint32_t)raw[1] <<  4) |
            (raw[2] >> 4);

    /* Cong thuc bu tru ap suat (datasheet Bosch BMP280, muc 4.2.3) */
    var1 = ((int64_t)s_t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)s_calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)s_calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)s_calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)s_calib.dig_P3) >> 8) +
           ((var1 * (int64_t)s_calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) *
            ((int64_t)s_calib.dig_P1) >> 33;

    if (var1 == 0)
        return 0.0f; /* Tranh chia cho 0 */

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)s_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)s_calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)s_calib.dig_P7) << 4);

    /* Ket qua hPa, do chinh xac 0.01 hPa */
    press = (float)p / 25600.0f;
    return press;
}

//      BME280_ReadAll

void BME280_ReadAll(uint8_t i2c_addr, BME280_Data *out)
{
    /* Doc nhiet do truoc (cap nhat s_t_fine) roi moi tinh ap suat */
    out->temperature = BME280_ReadTemperature(i2c_addr);
    out->pressure    = BME280_ReadPressure(i2c_addr);
}
