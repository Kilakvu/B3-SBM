#ifndef _escritura
#define _escritura

#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "Driver_SPI.h"


void copy_to_lcd_linea_1(void);		//COPIA EN LAS PAGINAS 0 Y 1
void copy_to_lcd_linea_2(void);		//COPIA EN LAS PAGINAS 2 Y 3
void wr_cmd(uint8_t cmd);				//PROCESO -> CS=0, A0=0, SEND CMD y CS=1
void wr_dat(uint8_t dat);				//PROCESO -> CS=0, A0=1, SEND DAT y CS=1
void init (void);							//INICIALIZACION
void lcd_reset(void);					//RESET


#endif

