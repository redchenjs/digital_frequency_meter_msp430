/*
 * lcd12864.c
 *
 *  Created on: 2016-06-25
 *      Author: Jack Chen
 *
 *      功能说明: LCD12864硬件驱动函数,包括LCD初始化,显示文字等功能。
 *      引脚配置: RS->P6.6, RW->P6.5, EN->P6.4, DATA->P3
 */

#include "msp430.h"

#include "lcd/lcd12864.h"

#define CPU_FREQ ((double)16000000)
#define delay_ms(x) __delay_cycles((long)(CPU_FREQ*(double)x/1000.0))

#define BUSY_FLAG 0x80

#define SET_CMD_PORT P6DIR |= 0x70; P8DIR |= BIT2; P8OUT |= BIT2
                                    // 设置命令口方向
#define SET_DATA_IN  P3DIR = 0x00   // 切换数据口方向,输入模式
#define SET_DATA_OUT P3DIR = 0xff   // 切换数据口方向,输出模式
#define LCD_DATA_IN  P3IN           // 选择LCD数据口输入寄存器
#define LCD_DATA_OUT P3OUT          // 选择LCD数据口输出寄存器
#define LCD_RS_H  P6OUT |= BIT6     // RS寄存器选择,选中数据寄存器
#define LCD_RS_L  P6OUT &=~BIT6     // RS寄存器选择,选中指令寄存器
#define LCD_RW_H  P6OUT |= BIT5     // RW读写位设置,选中读模式
#define LCD_RW_L  P6OUT &=~BIT5     // RW读写位设置,选中写模式
#define LCD_EN_H  P6OUT |= BIT4     // EN使能信号,使能有效
#define LCD_EN_L  P6OUT &=~BIT4     // EN使能信号,使能无效

void WaitForReady(void)
{
    unsigned char ReadTemp = 0;

    LCD_RS_L;                       // 选中指令寄存器
    LCD_RW_H;                       // 选中读取模式
    SET_DATA_IN;                    // 切换数据口到输入模式

    do {
        LCD_EN_H;                   // 使能打开
        _NOP();                     // 等待更新
        ReadTemp = LCD_DATA_IN;     // 读取数据口
        LCD_EN_L;                   // 使能关闭
    } while (ReadTemp & BUSY_FLAG); // 判忙等待
}

void SendCMD(unsigned char data)
{
    WaitForReady();                 // 判忙等待

    SET_DATA_OUT;                   // 切换数据口到输出模式

    LCD_RS_L;                       // 选中指令寄存器
    LCD_RW_L;                       // 选中写入模式
    LCD_DATA_OUT = data;            // 更新数据口

    LCD_EN_H;                       // 使能打开
    _NOP();                         // 等待更新
    LCD_EN_L;                       // 使能关闭
}

void SendData(unsigned char data)
{
    WaitForReady();                 // 判忙等待

    SET_DATA_OUT;                   // 切换数据口到输出模式

    LCD_RS_H;                       // 选中数据寄存器
    LCD_RW_L;                       // 选中写入模式
    LCD_DATA_OUT = data;            // 更新数据口

    LCD_EN_H;                       // 使能打开
    _NOP();                         // 等待更新
    LCD_EN_L;                       // 使能关闭
}

/*
 * 设置当前显存游标地址
 *
 * x: 1-8(每个地址两个字节)
 * y: 1-4(4行)
 */
void SetCoord(unsigned char x, unsigned char y)
{
   switch (y) {
        case 1: SendCMD(0x7F + x); break;
        case 2: SendCMD(0x8F + x); break;
        case 3: SendCMD(0x87 + x); break;
        case 4: SendCMD(0x97 + x); break;
        default:                   break;
   }
}

void WriteGDRAM(unsigned char data)
{
    unsigned char i,j,k;
    unsigned char bGDRAMAddrX = 0x80;   // GDRAM水平地址
    unsigned char bGDRAMAddrY = 0x80;   // GDRAM垂直地址
    for (i=0;i<2;i++) {
        for (j=0;j<32;j++) {
            for (k=0;k<8;k++) {
                SendCMD(0x34);          // 设置为8位MPU接口,扩充指令集,绘图模式关
                SendCMD(bGDRAMAddrY+j); // 垂直地址Y
                SendCMD(bGDRAMAddrX+k); // 水平地址X
                SendData(data);
                SendData(data);
            }
        }
        bGDRAMAddrX = 0x88;
    }
    SendCMD(0x36);              // 打开绘图模式
    SendCMD(0x30);              // 恢复基本指令集,关闭绘图模式
}

void LCD_Disp_String(char *str, unsigned char x, unsigned char y)
{
    unsigned char temp;
    SendCMD(0x30);
    SetCoord(x, y);
    temp = *str;
    while (temp != 0) {
        SendData(temp);
        temp = *++str;
    }
}

void LCD_CLR_GDRAM(void)
{
    WriteGDRAM(0x00);
}

void LCD_Fill_GDRAM(void)
{
    WriteGDRAM(0xff);
}

void LCD_CLR_DDRAM(void)
{
    SendCMD(0x01);
    SendCMD(0x34);
    SendCMD(0x30);
}

void LCD_Disp_Init(void)
{
    SET_CMD_PORT;       // 初始化命令口

    delay_ms(500);
    SendCMD(0x30);      // 基本指令集
    delay_ms(1);
    SendCMD(0x02);      // 地址归位
    delay_ms(1);
    SendCMD(0x0c);      // 整体显示打开,游标关闭
    delay_ms(1);
    SendCMD(0x01);      // 清除显示
    delay_ms(10);
    SendCMD(0x06);      // 游标右移
    delay_ms(1);
    SendCMD(0x80);      // 设定显示的起始地址

    LCD_CLR_GDRAM();
    LCD_CLR_DDRAM();
}
