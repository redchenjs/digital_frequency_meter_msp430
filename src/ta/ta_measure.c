/*
 * ta_measure.c
 *
 *  Created on: 2016-05-24
 *      Author: Jack Chen
 *
 *      功能说明: Timer_A同步等精度测量法及周期测量法测量部分及数据处理相关函数。
 *      引脚配置:
 *              P2.2(输入口): 同步测量模式中TA2CLK输入，连接CMP_OUT待测信号
 *              P2.0(输入口): 周期测量模式中TA1.1捕获输入，连接CMP_OUT待测信号
 *              P1.5(输入口): 同步测量模式中D触发器Q输出，连接74HC74输出口Q
 *              P1.4(输出口): 同步测量模式中D触发器D输入，连接74HC74数据口D
 */

#include "stdio.h"

#include "msp430.h"

#include "ta/ta_init.h"
#include "lcd/lcd12864.h"

#define CLK_FREQ    4000000

#define SET_SYNC_IN P1DIR &=~BIT5; P1IES &=~BIT5; P1IE |= BIT5
#define CLR_SYNC_IN P1DIR &=~BIT5; P1IE  &=~BIT5

#define SYNC_H      P1DIR |= BIT4; P1OUT |= BIT4
#define SYNC_L      P1DIR |= BIT4; P1OUT &=~BIT4

#define LED_ON      P4DIR |= BIT7; P4OUT |= BIT7
#define LED_OFF     P4DIR |= BIT7; P4OUT &=~BIT7

#define SIG_ADJUST  0.0000666                   // SIG校准数据
#define CLK_ADJUST  0.0000515                   // CLK校准数据

unsigned char mode_flag = 1;                    // 测量模式标志位

unsigned long sig_count = 0;                    // SIG计数值
unsigned long clk_count = 0;                    // CLK计数值

double freq_result = 0;                         // 频率计算结果

unsigned char first_flag = 0;
unsigned int r_edge1 = 0, r_edge2 = 0, f_edge = 0;
unsigned long period = 0;

void MeasFreq(void)
{
    LED_ON;                                     // 开LED指示灯

    sig_count = 0;
    clk_count = 0;
    freq_result = 0;

    TA_Timer_Start();                           // 启动定时器

    switch (mode_flag) {
        case 0:
            SET_SYNC_IN;                        // 设置同步信号输入口

            TA_Count_Init();                    // 初始化计数模式
            TA_Count_Start();                   // 启动计数器

            SYNC_H;                             // 输出同步信号

            __bis_SR_register(CPUOFF + GIE);    // 等待测量

            SYNC_L;                             // 关闭同步信号
            break;
        case 1:
            first_flag = 0;
            r_edge1 = 0;
            r_edge2 = 0;
            f_edge = 0;
            period = 0;

            TA_Capture_Init();                  // 初始化捕获模式
            TA_Capture_Start();                 // 启动捕获
            __bis_SR_register(CPUOFF + GIE);    // 等待测量
            TA_Capture_Stop();                  // 停止捕获
            break;
    }
}

void CalcFreq(void)
{
    clk_count *= 65535;                         // 恢复实际计数值
    sig_count *= 65535;

    switch (mode_flag) {
        case 0:
            clk_count += TA1R;                  // 恢复实际计数值
            sig_count += TA2R;

            clk_count += (unsigned long)(CLK_ADJUST * clk_count);       // 加入误差调整
            sig_count += (unsigned long)(SIG_ADJUST * sig_count);

            if (clk_count) {
                freq_result = (double)sig_count / clk_count * CLK_FREQ; // 计算
            } else {
                freq_result = 0.0;
            }
            break;
        case 1:
            period = clk_count + r_edge2 - r_edge1;     // 计算捕获周期
            freq_result = (double)CLK_FREQ / period;    // 计算
            break;
    }

    if (freq_result < 10001) {                          // 检测频率切换测量模式
        mode_flag = 1;
    } else {
        mode_flag = 0;
    }
}

void DispFreq(void)
{
    char freq[17]={0};
    char sigc[17]={0};
    char clkc[17]={0};
    char mode[17]={0};

    if (freq_result < 10) {                             // Hz
        sprintf(freq, "freq:%-9.7fHz", freq_result);    // 低于10Hz,显示单位Hz，7位小数，1.000 000 0Hz
    } else if (freq_result < 100) {
        sprintf(freq, "freq:%-9.6fHz", freq_result);    // 低于100Hz，显示单位Hz，6位小数，10.000 000Hz
    } else if (freq_result < 1000) {
        sprintf(freq, "freq:%-9.5fHz", freq_result);    // 低于1kHz,显示单位Hz，5位小数,100.000 00Hz
    } else if (freq_result < 10000) {                   // kHz
        freq_result /= 1000;
        sprintf(freq, "freq:%-8.6fkHz", freq_result);   // 低于10kHz,显示单位kHz，6位小数，1.000 000kHz
    } else if (freq_result < 100000) {
        freq_result /= 1000;
        sprintf(freq, "freq:%-8.5fkHz", freq_result);   // 低于100kHz,显示单位kHz，5位小数,10.000 00kHz
    } else if (freq_result < 1000000) {
        freq_result /= 1000;
        sprintf(freq, "freq:%-8.4fkHz", freq_result);   // 低于1MHz,显示单位kHz，4位小数,100.000 0kHz
    } else {                                            // MHz
        freq_result /= 1000000;
        sprintf(freq, "freq:%-8.6fMHz", freq_result);   // 低于10MHz,显示单位MHz，6位小数,1.000 000MHz
    }

    LCD_CLR_GDRAM();                                    // 清除LCD图形显存

    sprintf(sigc, "sig:%-12lu", sig_count);             // 字符格式化处理
    sprintf(clkc, "clk:%-12lu", clk_count);

    if (mode_flag) {
        sprintf(mode, "mode:period ");
    } else {
        sprintf(mode, "mode:synchro");
    }

    LCD_Disp_String(sigc, 1, 1);                        // 输出结果
    LCD_Disp_String(clkc, 1, 2);
    LCD_Disp_String(freq, 1, 3);
    LCD_Disp_String(mode, 1, 4);
}

void TA_Measure(void)
{
    MeasFreq();                                 // 测量
    CalcFreq();                                 // 计算频率
    DispFreq();                                 // 输出结果
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
{
    switch (__even_in_range(TA1IV,0x0A)) {
        case  TA1IV_TACCR1:
            if (!first_flag) {
                r_edge1 = TA1CCR1;              // 第一次捕获到上升沿的计数值
                first_flag++;
            }
            else {
                r_edge2 = TA1CCR1;              // 第二次捕获到上升沿的计数值
                first_flag = 0;
                __bic_SR_register_on_exit(CPUOFF + GIE);
            }
            break;
        case TA1IV_TAIFG:
            TA1CTL &=~TAIFG;                    // 手动清除TA标志位
            clk_count++;                        // 计CLK溢出次数
            break;
    }
}
#pragma vector = TIMER2_A1_VECTOR
__interrupt void TIMER2_A1_ISR(void)
{
    TA2CTL &=~TAIFG;                            // 手动清除TA标志位
    sig_count++;                                // 计SIG溢出次数
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    LED_OFF;                                    // 关灯

    __bic_SR_register_on_exit(CPUOFF + GIE);    // 唤醒CPU
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    switch (__even_in_range(P1IV,0x10)) {
        case P1IV_P1IFG5:
            P1IFG = 0;                          // 清除P1口中断标志位
            if (P1IES & BIT5) {                 // 下降沿说明本次测量结束，停止计数
                TA_Count_Stop();                // 停止计数
                P1IES &=~BIT5;                  // 切换到上升沿中断
            } else {                            // 上升沿说明信号已同步，清空计数器开始计数
                P1IES |= BIT5;                  // 切换到下降沿中断

                sig_count=0;                    // 清空计数器
                clk_count=0;
                TA1R=0;
                TA2R=0;
            }
            break;
        default:
            break;
    }
}
