#ifndef _joystick
#define _joystick

#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#define SUBIDA 0x0001
#define BAJADA 0x0002

//---------CONFIGURACIÓN DEL JOYSTICK-------
#define PUERTO_JOY 0
#define ABAJO	17				//DIN 12
#define IZQUIERDA	15				//DIN 13
#define CENTRO	16			//NO USADO
#define ARRIBA	23					//DIN 15
#define DERECHA	24				//DIN 16
//---------------------------------------
void EINT3_IRQHandler (void);	//Interrupcion
void inicio_joystick(void);	//Inicializacion
void flag_joystick(void);	//Lectura de pulsacion
#endif
