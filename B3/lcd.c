#include "lcd.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "Driver_SPI.h"
#include <stdio.h>
#include <string.h>
#include "Arial12x12.h"
#include "Letra.h"
#include "escritura.h"
uint8_t buffer[512];
uint16_t posicionL1 = 0;
uint16_t posicionL2 = 0;
int EscribirTEST(uint8_t letra)
{
	uint8_t flagR=0x52;
	uint8_t flagL=0x4C;
	uint8_t flagF=0x46;
	uint16_t comienzo = 0;
	posicionL1=50;	
	uint8_t i;
	if(letra == flagR)
	{
		comienzo = 91;
	for(i=0;i<45;i++){
		buffer[i+128+posicionL1] = Letra[comienzo+i*2+1];
		buffer[i+256+posicionL1] = Letra[comienzo+i*2+2];
	}		
	comienzo = 364;
		for(i=0;i<45;i++){
		buffer[i+posicionL1] = Letra[comienzo+i*2+1];
		buffer[i+384+posicionL1] = Letra[comienzo+i*2+2];	
		}
	}
	if(letra == flagL)
	{
		comienzo = 0;	
	for(i=0;i<45;i++){
		buffer[i+128+posicionL1] = Letra[comienzo+i*2+1];
		buffer[i+256+posicionL1] = Letra[comienzo+i*2+2];
	}	
		comienzo = 364;
		for(i=0;i<45;i++){
		buffer[i+posicionL1] = Letra[comienzo+i*2+1];
		buffer[i+384+posicionL1] = Letra[comienzo+i*2+2];	
		}
	}
		
	if(letra == flagF)
	{
	
		comienzo = 182;
		for(i=0;i<45;i++){
		buffer[i+128+posicionL1] = Letra[comienzo+i*2+1];
		buffer[i+posicionL1] = Letra[comienzo+i*2+2];
		
		}
		comienzo = 273;
		for(i=0;i<45;i++){
		buffer[i+256+posicionL1] = Letra[comienzo+i*2+1];
		buffer[i+384+posicionL1] = Letra[comienzo+i*2+2];
			
	}
		
	}
	
	copy_to_lcd_linea_1();
	copy_to_lcd_linea_2();
	return 0;
}

int EscribeLetra_L1(uint8_t letra){
	
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;		
	comienzo = 25 *(letra - ' ');		//EN EL ARIAL12X12 BUSCA EL INICIO DE SU LINEA
	
	for(i=0;i<12;i++){
		valor1 = Arial12x12[comienzo+i*2+1];	//NO COJE EL PRIMER VALOR ? Y LOS AÑADE SALTEADOS
		valor2 = Arial12x12[comienzo+i*2+2]; //NO COJE EL PRIMER VALOR ? Y LOS AÑADE SALTEADOS
		
		buffer[i+posicionL1] = valor1;
		buffer[i+128+posicionL1] = valor2;
	}
	posicionL1= posicionL1 + Arial12x12[comienzo];
	
	return 0;
}

int EscribeLetra_L2(uint8_t letra2){
	
	uint8_t i, valor1, valor2;
	uint16_t comienzo = 0;
	comienzo = 25 *(letra2 - ' ');
	
	for(i=0;i<12;i++){
		valor1 = Arial12x12[comienzo+i*2+1];
		valor2 = Arial12x12[comienzo+i*2+2];
		
		buffer[i+256+posicionL2] = valor1;
		buffer[i+384+posicionL2] = valor2;
	}
	posicionL2= posicionL2 + Arial12x12[comienzo];
	
	return 0;
}

int EscribeFrase(char cadena[],int pagina){
	

	posicionL1=0;
	posicionL2=0;
	uint8_t i, j;
	int longitud = strlen(cadena);
	if (pagina == 1){
		for(i=0; i < longitud; i++)
		{
						EscribeLetra_L1(cadena[i]);

		}
	}
	
	if (pagina == 2 ){
		for(j=0; j < longitud; j++)
		{
			EscribeLetra_L2(cadena[j]);
		}
	}
	copy_to_lcd_linea_1();
	copy_to_lcd_linea_2();
	return 0;
}




int clear_lcd(void)
{
	uint8_t i;
	posicionL1=0;
	posicionL2=0;
		for(i=0; i < 128; i++)
		{
		buffer[i+posicionL1] = 0x00;
		buffer[i+128+posicionL1] = 0x00;
		buffer[i+256+posicionL2] = 0x00;
		buffer[i+384+posicionL2] = 0x00;
		}
	copy_to_lcd_linea_1();
	copy_to_lcd_linea_2();
  return 0;
}


