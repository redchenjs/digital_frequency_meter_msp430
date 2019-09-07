/*
 * lcd12864.h
 *
 *  Created on: 2015-06-25
 *      Author: Jack Chen
 */

#ifndef INC_LCD12864_H_
#define INC_LCD12864_H_

extern void LCD_CLR_DDRAM(void);
extern void LCD_CLR_GDRAM(void);

extern void LCD_Disp_String(char *str, unsigned char x, unsigned char y);

extern void LCD_Disp_Init(void);

#endif /* INC_LCD12864_H_ */
