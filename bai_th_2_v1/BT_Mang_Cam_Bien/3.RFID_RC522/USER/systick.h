/**
 * @file    systick.h
 * @brief   Thu vien Delay chinh xac su dung SysTick (Cortex-M3)
 *          Khong can Timer peripheral, chinh xac 1ms
 *
 * Su dung:
 *   Goi SysTick_Init() 1 lan dau trong main()
 *   Sau do goi Delay_ms(n) o bat cu dau
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include "define.h"

//===========================//
//          API              //
//===========================//

/**
 * @brief  Khoi dong SysTick voi SYSCLK = 72MHz
 *         Cau hinh ngat moi 1ms, bat dau dem
 */
void SysTick_Init(void);

/**
 * @brief  Delay chinh xac theo ms (dung SysTick)
 * @param  ms  So mili giay can delay
 */
void Delay_ms(uint32_t ms);

#endif /* SYSTICK_H */
