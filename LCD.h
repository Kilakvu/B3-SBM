#ifndef __LCD_H
#define __LCD_H

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

int LCD_reset(void);
void lcd_clean(void);
void copy_to_lcd(void);
void wr_data(unsigned char data);
void wr_cmd(unsigned char cmd);
int EscribeLetra_L1 (uint8_t letra);
int EscribeLetra_L2 (uint16_t letra);
void actualizar(char lcd_text[2][20+1]);
void GPIO_INIT(void);
void pant_neg (void);
void LCD_DrawBar(uint8_t x, uint8_t height);

#endif
