#include "systick.h"

static volatile uint32_t s_tick = 0;

void SysTick_Handler (void)
{
    s_tick++;
}

void SysTick_Init(void)
{
    // C?u hình ng?t SysTick m?i 1ms (SYSCLK = 72MHz)
    SysTick_Config(SystemCoreClock / 1000);
}

void delay_ms(uint32_t ms)
{
    uint32_t start = s_tick;
    while ((s_tick - start) < ms);
} 
