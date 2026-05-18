#include "dht11.h"

// Bo dem luu du lieu tu DHT11
uint8_t u8Buff[5] = {0};

void DHT11_Init(void)
{
    GPIO_InitTypeDef gpioInit;

    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpioInit.GPIO_Mode = GPIO_Mode_Out_OD;
    gpioInit.GPIO_Pin = GPIO_Pin_12;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioInit);

    
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

/** Bit 0: muc cao trong 26-28us
    Bit 1: muc cao trong ~70us
 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    uint8_t i;
    uint16_t u16Tim;

    for (i = 0; i < 8; i++) 
    {
        
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12));

        
        while (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12));

        TIM_SetCounter(TIM4, 0);

        
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) 
        {
            if (TIM_GetCounter(TIM4) > 100) break; // Timeout
        }

        u16Tim = TIM_GetCounter(TIM4);

        byte <<= 1;
        if (u16Tim > 40) 
        {
            byte |= 1; //  > 40us,  là bit 1
        }
    }

    return byte;
}


void DHT11_Read(void)
{
    uint8_t i;
    uint8_t u8CheckSum;
    uint16_t time_out = 0;

    // Keo muc thap 20ms de bao hieu bat dau doc
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    delay_ms(20);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    // Cho DHT11 phan hoi keo xuong
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12))
    {
        time_out++;
        if (time_out > 1000) 
        {
            flag = 0; // Loi timeout
            return;
        }
    }

    // Cho DHT11 keo len
    time_out = 0;
    while (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) 
    {
        time_out++;
        if (time_out > 1000) 
        {
            flag = 0;
            return;
        }
    }

    // Cho DHT11 keo xuong lan nua
    time_out = 0;
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12))
    {
        time_out++;
        if (time_out > 1000) 
        {
            flag = 0;
            return;
        }
    }


    for (i = 0; i < 5; i++) 
    {
        u8Buff[i] = DHT11_ReadByte();
    }

    u8CheckSum = u8Buff[0] + u8Buff[1] + u8Buff[2] + u8Buff[3];
    if (u8CheckSum != u8Buff[4]) 
    {
        flag = 0; // Loi checksum
        return;
    }

    flag = 1; // Doc du lieu thanh cong
}


float DHT11_Get_Temperature(void)
{
    return (float)u8Buff[2] + (float)u8Buff[3] / 10.0f;
}


float DHT11_Get_Humidity(void)
{
    return (float)u8Buff[0] + (float)u8Buff[1] / 10.0f;
}
