#include "define.h"

static MPU9265_Data_t mpu_raw;
static MPU9265_Phys_t mpu_phys;

int main(void) 
{
    SysTick_Init();
    Uart_Gpio_TxRx_Init();
    Uart_Init();
    SPI1_Config();
    
    Uart_SendStr("\n=== STM32 & MPU-6500/9265 ===\n");
    
    if (MPU9265_Init()) {
        Uart_SendStr("MPU san sang!\n\n");
    } else {
        Uart_SendStr("Loi k?t n?i SPI!\n");
        while(1); 
    }
    
    while(1) 
    {
        // Ð?c d? li?u thô
        MPU9265_Read_All(&mpu_raw);
        
        // Tính toán sang d?i lu?ng v?t lý
        MPU9265_Convert(&mpu_raw, &mpu_phys);
        
        // Hi?n th? Gia t?c
        Uart_SendStr("Gia toc (m/s2) | X: "); Uart_SendFloat(mpu_phys.Accel_X_ms2);
        Uart_SendStr(" | Y: ");               Uart_SendFloat(mpu_phys.Accel_Y_ms2);
        Uart_SendStr(" | Z: ");               Uart_SendFloat(mpu_phys.Accel_Z_ms2);
        Uart_SendStr("\n");
        
        // Hi?n th? T?c d? góc
        Uart_SendStr("Van toc (do/s) | X: "); Uart_SendFloat(mpu_phys.Gyro_X_dps);
        Uart_SendStr(" | Y: ");               Uart_SendFloat(mpu_phys.Gyro_Y_dps);
        Uart_SendStr(" | Z: ");               Uart_SendFloat(mpu_phys.Gyro_Z_dps);
        Uart_SendStr("\n---------------------------------\n");
        
        delay_ms(1000); 
    }
}
