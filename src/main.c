/*
 * main.c
 *
 *  Created on: 2016-07-07
 *      Author: Jack Chen
 */

#include "msp430.h"

#include "ta/ta_init.h"
#include "ta/ta_measure.h"
#include "ucs/ucs_init.h"
#include "lcd/lcd12864.h"

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;				// Stop watchdog timer

    UCS_Init();                 			// UCS时钟初始化

    TA_Timer_Init();						// 定时器初始化

    LCD_Disp_Init();						// LCD12864初始化

    while (1) {
        TA_Measure();						// 循环测量
        __bis_SR_register(CPUOFF + GIE);	// 待机休眠
    }
}
