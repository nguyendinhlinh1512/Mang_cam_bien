#ifndef __MPU9265_H
#define __MPU9265_H

#include "define.h"

// Thanh ghi MPU
#define MPU_REG_CONFIG          0x1A // loc nhieu
#define MPU_REG_GYRO_CONFIG     0x1B 
#define MPU_REG_ACCEL_CONFIG    0x1C
#define MPU_REG_USER_CTRL       0x6A // dk giao dien -> spi (k i2c)
#define MPU_REG_PWR_MGMT_1      0x6B // ghi 0x80 de Reset , ghi 0x01 de start
#define MPU_REG_WHO_AM_I        0x75
#define MPU_REG_ACCEL_XOUT_H    0x3B

// === Ð?NH NGHIA MACRO D?I ÐO CHU?N VI M?CH ===
#define ACCEL_FS_2G             (0 << 3)
#define ACCEL_FS_4G             (1 << 3)
#define ACCEL_FS_8G             (2 << 3) // Tuong duong 0x10
#define ACCEL_FS_16G            (3 << 3)

#define GYRO_FS_250             (0 << 3)
#define GYRO_FS_500             (1 << 3)
#define GYRO_FS_1000            (2 << 3) // Tuong duong 0x10
#define GYRO_FS_2000            (3 << 3)

// Struct luu d? li?u RAW (Raw Data)
typedef struct {
    int16_t Accel_X;
    int16_t Accel_Y;
    int16_t Accel_Z;
    int16_t Gyro_X;
    int16_t Gyro_Y;
    int16_t Gyro_Z;
} MPU9265_Data_t;

// Struct luu d? li?u V?t lý (m/s^2 và d?/s)
typedef struct {
    float Accel_X_ms2;
    float Accel_Y_ms2;
    float Accel_Z_ms2;
    float Gyro_X_dps;
    float Gyro_Y_dps;
    float Gyro_Z_dps;
} MPU9265_Phys_t;

uint8_t MPU9265_Init(void);
void MPU9265_Read_All(MPU9265_Data_t *mpuData);
void MPU9265_Convert(MPU9265_Data_t *rawData, MPU9265_Phys_t *physData);

#endif
