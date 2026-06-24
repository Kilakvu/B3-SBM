#ifndef _lcd
#define _lcd

#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "Driver_SPI.h"
#include <stdio.h>
#include <stdio.h>
#include <string.h>

int EscribeLetra_L1(uint8_t letra);	//ESCRIBE LETRA POR LETRA EN LAS PAGINAS 0 Y 1
int EscribeLetra_L2(uint8_t letra2); //ESCRIBE LETRA POR LETRA EN LAS PAGINAS 2 Y 3
int clear_lcd(void);			//LIMPIA EL LCD
int EscribeFrase(char cadena[], int pagina); //ESCRIBE FRASES INCLUIDAS COMO UNA CADENA
																				//DE CHAR Y QUE INCLUYE A ESCRBIR LETRA POR LETRA

int EscribirTEST(uint8_t letra2);


#endif

