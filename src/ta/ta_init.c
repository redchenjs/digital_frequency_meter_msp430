/*
 * ta_init.c
 *
 *  Created on: 2016-04-11
 *      Author: Jack Chen
 *
 *      功能说明: Timer_A相关功能初始化设置,包括捕获模式,定时器,计数器初始化配置。
 *      引脚配置:
 *      		 P2.2(输入口): 同步测量模式中TA2CLK输入,连接CMP_OUT待测信号
 *      		 P2.0(输入口): 周期测量模式中TA1.1捕获输入,连接CMP_OUT待测信号
 *      		 P1.4(输出口): 同步测量模式中D触发器D输入,连接74HC74数据口D
 */

#include "msp430.h"

#define SET_CNT_PORT    P2DIR &=~BIT2; P2SEL |= BIT2; P1DIR |= BIT4; P1OUT &=~BIT4
#define CLR_CNT_PORT    P2DIR &=~BIT2; P2SEL &=~BIT2
#define SET_CAP_PORT    P2DIR &=~BIT0; P2SEL |= BIT0
#define CLR_CAP_PORT    P2DIR &=~BIT0; P2SEL &=~BIT0

void TA_Count_Init(void)
{
    CLR_CAP_PORT;
    SET_CNT_PORT;                           // 初始化SIG接口

    TA1CTL = TASSEL_2 + MC_0 + TAIE;        // CLK计数器配置: SMCLK,停止计数,开启TA中断

    TA2CTL = TASSEL_0 + MC_0 + TAIE;        // SIG计数器配置: TACLK,停止计数,开启TA中断
}

void TA_Count_Start(void)
{
    TA1CTL |= MC_2 + TACLR;                 // CLK计数器配置: 连续计数,清计数器

    TA2CTL |= MC_2 + TACLR;                 // SIG计数器配置: 连续计数,清计数器
}

void TA_Count_Stop(void)
{
    TA1CTL &=~MC_2;                         // CLK计数器配置: 停止计数
    TA1CTL |= MC_0;

    TA2CTL &=~MC_2;                         // SIG计数器配置: 停止计数
    TA2CTL |= MC_0;
}

void TA_Timer_Init(void)
{
    TA0CCR0  = 16383;                       // TA0定时器配置: 0.5s定时
    TA0CTL 	 = TASSEL_1 + MC_0;             // TA0定时器配置: ACLK,停止计数
    TA0CCTL0 = CCIE;                        // TA0定时器配置: 开启中断
}

void TA_Timer_Start(void)
{
    TA0CTL |= MC_1 + TACLR;                 // TA0定时器配置: 增计数,清计数器
}

void TA_Timer_Stop(void)
{
    TA0CTL &=~MC_1;                         // TA0定时器配置: 停止计数
    TA0CTL |= MC_0;
}

void TA_Capture_Init(void)
{
    CLR_CNT_PORT;
    SET_CAP_PORT;                           // 初始化捕获接口

    TA1CCTL1 = CAP + CM_1 + SCS;            // TA1.1寄存器配置: 捕获模式,上升沿捕获,同步捕获
    TA1CTL   = TASSEL_2 + MC_2;             // TA1寄存器配置: SMCLK,连续计数
}

void TA_Capture_Start(void)
{
    TA1CCTL1 |= CCIE;
    TA1CTL   |= TAIE + TACLR;               // TA1寄存器配置: 开启中断,清计数器
}

void TA_Capture_Stop(void)
{
    TA1CCTL1 &=~CCIE;
    TA1CTL   &=~TAIE;                       // TA1寄存器配置: 关闭中断
}
