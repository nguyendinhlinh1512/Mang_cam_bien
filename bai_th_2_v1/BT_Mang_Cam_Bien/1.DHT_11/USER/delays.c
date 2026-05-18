#include "delays.h"

void Timer4_Init(void)
{
	// Cap xung nhip cho TIM4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseInitTypeDef timerInit; 

	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_Period = 0xFFFF;

	// He so chia (Prescaler) = 72 - 1 (de tao xung 1 MHz neu su dung clock 72 MHz)
	timerInit.TIM_Prescaler = 72 - 1;
	timerInit.TIM_ClockDivision = 0;

	TIM_TimeBaseInit(TIM4, &timerInit);
	TIM_Cmd(TIM4, ENABLE);
}

void Delay1Ms(void)
{
	TIM_SetCounter(TI2M4, 0);

	//  du 1000 (ung voi 1 ms voi tan so 1 MHz)
	while (TIM_GetCounter(TIM4) < 1000) 
	{
	}
}

void delay_ms(uint32_t u32DelayInMs)
{
	// Lap lai Delay1Ms() voi so lan tuong ung voi u32DelayInMs
	while (u32DelayInMs) 
	{
		Delay1Ms();
		--u32DelayInMs;
	}
}
