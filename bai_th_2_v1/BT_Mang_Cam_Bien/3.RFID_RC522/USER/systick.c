/**
 * @file    systick.c
 * @brief   Thu vien Delay chinh xac su dung SysTick (Cortex-M3)
 *          SYSCLK = 72MHz -> SysTick dem tu 72000 -> ngat moi 1ms
 */

#include "systick.h"

//===========================//
//   BIEN DEM THOI GIAN      //
//===========================//

static volatile uint32_t s_tick = 0;  /* Tang len moi 1ms trong ISR */

//===========================//
//     SysTick_Handler       //
//===========================//

/**
 * @brief  ISR cua SysTick - tu dong goi moi 1ms
 *         Khong can khai bao them, ten ham da duoc dinh nghia
 *         san trong CMSIS (startup file)
 */
void SysTick_Handler(void)
{
    s_tick++;
}

//===========================//
//      SysTick_Init         //
//===========================//

void SysTick_Init(void)
{
    /*
     * SysTick_Config() la ham CMSIS chuan (core_cm3.h)
     * Tham so: so chu ky clock giua moi ngat
     * SystemCoreClock = 72000000 Hz
     * SystemCoreClock / 1000 = 72000 chu ky -> ngat moi 1ms
     */
    SysTick_Config(SystemCoreClock / 1000);
}

//===========================//
//        Delay_ms           //
//===========================//

void Delay_ms(uint32_t ms)
{
    uint32_t start = s_tick;

    /* Cho den khi du so ms da troi qua */
    while ((s_tick - start) < ms);
}
