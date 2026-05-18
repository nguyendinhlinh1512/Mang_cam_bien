#include "tim2.h"

static volatile uint32_t counter_ms = 0;

void Timer2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->PSC  = 72 - 1;     // 72MHz / 72 = 1MHz -> 1 tick = 1us
    TIM2->ARR  = 1000 - 1;   // 1000us = 1ms -> ngat moi 1ms
    TIM2->CNT  = 0;

    TIM2->DIER |= TIM_DIER_UIE;  // Bat ngat update
    TIM2->CR1  |= TIM_CR1_CEN;   // Bat timer

    NVIC_SetPriority(TIM2_IRQn, 1);
    NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;
        counter_ms++;
    }
}

uint32_t millis(void)
{
    return counter_ms;
}

void Delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms);
}


void Delay_us(uint16_t us)
{
    // Voi clock 72MHz, 1us tan khoang 72 chu ki máy.
    // M?t vòng l?p while(count--) t?n kho?ng 4-5 chu k? máy.
    volatile uint32_t count = us * 15; 
    while (count--);
}