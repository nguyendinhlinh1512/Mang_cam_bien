#include "define.h"

static void Delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    for (; ms > 0; ms--)
        for (i = 0; i < 7200; i++);
}

int main(void)
{
    BME280_Data sensor;

    SystemInit();

    /* Khoi dong UART */
    Uart_Gpio_TxRx_Init();
    Uart_Init();

    /* Khoi dong I2C */
    I2C1_Config();

    /* Khoi dong cam bien */
    Uart_SendStr("=== BME280 DEMO ===\r\n");

    if (!BME280_Init(BME280_I2C_ADDR_DEFAULT))
    {
        Uart_SendStr("[LOI] Khong tim thay cam bien!\r\n");
        while (1);
    }

    Uart_SendStr("[OK] Cam bien san sang. Bat dau do...\r\n");
    Uart_SendStr("--------------------------\r\n");

    while (1)
    {
        BME280_ReadAll(BME280_I2C_ADDR_DEFAULT, &sensor);

        Uart_SendStr("Nhiet do : ");
        Uart_SendFloat(sensor.temperature);
        Uart_SendStr(" C\r\n");

        Uart_SendStr("Ap suat  : ");
        Uart_SendFloat(sensor.pressure);
        Uart_SendStr(" hPa\r\n");

        Uart_SendStr("--------------------------\r\n");

        Delay_ms(2000);
    }
}
