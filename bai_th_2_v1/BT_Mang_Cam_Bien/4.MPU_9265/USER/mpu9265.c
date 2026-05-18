#include "mpu9265.h"

static void MPU_WriteReg(uint8_t reg, uint8_t data) {
    SPI1_CS_Low();
    SPI1_TransmitReceive(reg & 0x7F); 
    SPI1_TransmitReceive(data);
    SPI1_CS_High();
}

static uint8_t MPU_ReadReg(uint8_t reg) {
    uint8_t value;
    SPI1_CS_Low();
    SPI1_TransmitReceive(reg | 0x80); 
    value = SPI1_TransmitReceive(0x00);
    SPI1_CS_High();
    return value;
}

static void MPU_ReadRegs(uint8_t reg, uint8_t *buffer, uint8_t len) {
    SPI1_CS_Low();
    SPI1_TransmitReceive(reg | 0x80);
    for (uint8_t i = 0; i < len; i++) {
        buffer[i] = SPI1_TransmitReceive(0x00);
    }
    SPI1_CS_High();
}

uint8_t MPU9265_Init(void)
{
    uint8_t id = MPU_ReadReg(MPU_REG_WHO_AM_I);
    
    // In ID ra d? ki?m tra
    Uart_SendStr("-> MPU ID: ");
    Uart_SendInt(id);
    Uart_SendStr("\n");
    
    // Ðã thêm 0x70 (112) d? ch?p nh?n lõi MPU-6500
    if (id != 0x71 && id != 0x73 && id != 0x70) {
        return 0; 
    }

    MPU_WriteReg(MPU_REG_PWR_MGMT_1, 0x80); 
    delay_ms(100);
    MPU_WriteReg(MPU_REG_PWR_MGMT_1, 0x01);
    MPU_WriteReg(MPU_REG_USER_CTRL, 0x10);
    
    // S? d?ng Macro thay vì vi?t 0x10 tr?c ti?p
    MPU_WriteReg(MPU_REG_ACCEL_CONFIG, ACCEL_FS_8G);
    MPU_WriteReg(MPU_REG_GYRO_CONFIG, GYRO_FS_1000);

    return 1;
}

void MPU9265_Read_All(MPU9265_Data_t *mpuData)
{
    uint8_t raw[14];
    MPU_ReadRegs(MPU_REG_ACCEL_XOUT_H, raw, 14);

    mpuData->Accel_X = (int16_t)((raw[0] << 8) | raw[1]);
    mpuData->Accel_Y = (int16_t)((raw[2] << 8) | raw[3]);
    mpuData->Accel_Z = (int16_t)((raw[4] << 8) | raw[5]);
    
    mpuData->Gyro_X  = (int16_t)((raw[8] << 8) | raw[9]);
    mpuData->Gyro_Y  = (int16_t)((raw[10] << 8) | raw[11]);
    mpuData->Gyro_Z  = (int16_t)((raw[12] << 8) | raw[13]);
}

// Hàm quy d?i t? Raw sang V?t lý
void MPU9265_Convert(MPU9265_Data_t *rawData, MPU9265_Phys_t *physData)
{
    // H? s? chia cho +-8g là 4096 LSB/g. Tr?ng l?c 1g = 9.81 m/s^2
    physData->Accel_X_ms2 = ((float)rawData->Accel_X / 4096.0f) * 9.81f;
    physData->Accel_Y_ms2 = ((float)rawData->Accel_Y / 4096.0f) * 9.81f;
    physData->Accel_Z_ms2 = ((float)rawData->Accel_Z / 4096.0f) * 9.81f;

    // H? s? chia cho +-1000 dps là 32.8 LSB/(do/s)
    physData->Gyro_X_dps = (float)rawData->Gyro_X / 32.8f;
    physData->Gyro_Y_dps = (float)rawData->Gyro_Y / 32.8f;
    physData->Gyro_Z_dps = (float)rawData->Gyro_Z / 32.8f;
}
