#include "debug.h"

uint8_t flag = 0;


/**
 * @brief Init LED for debug GPIOC | PC13
 * 
 */
void Led_Debug(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE); // clock 
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; //pc13
	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
 * @brief turn on led
 * 
 */
void on(void)
{    
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}


/**
 * @brief turn off led
 * 
 */
void off(void)
{
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}


/**
 * @brief blink led for a number of times with 1s per blink
 * 
 * @param times 
 */
void Blink_Times(uint8_t times)
{
    for(uint8_t i = 0; i < times; i++)
    {
        on();
        delay_ms(500);
        off();
        delay_ms(500);
    }
}


/**
 * @brief Debug function to blink LED based on flag value
 * 
 */
void Debug(void)
{
    switch(flag)
    {
        case 1:
            Blink_Times(1);
            break;
        case 2:
            Blink_Times(2);
            break;
        case 3:
            Blink_Times(3);
            break;
        default:
            off();
            break;
    }
    flag = 0; // Reset flag after executing pattern
}