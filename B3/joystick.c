#include "PIN_LPC17xx.h"
#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include <stdio.h>
#include <string.h>
//---------CONFIGURACION DE LOS LEDS------------------------
#define PUERTO_LED1   1
#define LED_1   18
#define LED_2   20
#define LED_3   21
#define LED_4   23


//---------CONFIGURACIÓN DEL JOYSTICK-------
#define PUERTO_JOY 0
#define ABAJO	17				//DIN 12
#define IZQUIERDA	15				//DIN 13
#define CENTRO	16			//NO USADO
#define ARRIBA	23					//DIN 15
#define DERECHA	24				//DIN 16
//---------------------------------------
#include "retardo.h"

#define SUBIDA 0x0001
#define BAJADA 0x0002
#define NORMAL 0
#define TEST 1
#define DESARMADO 2

extern char estado[15];
extern int flagEstado;
uint32_t valor;
extern float guarda;
extern float tmepRef;
/*---------------------------------------------------------
*							Acciones que realiza el JOystick
*							Algunas caracteristicas no funcionan en 
*							otro estado que no sea NORMAL
*----------------------------------------------------------*/
void flag_joystick(void)
{
	
	if (valor== 1 << ARRIBA && flagEstado == NORMAL)
	{
		tmepRef=tmepRef+0.5;
	}
		if (valor== 1 << ABAJO && flagEstado == NORMAL)
	{
		tmepRef=tmepRef-0.5;
	}
	if (valor== 1 << DERECHA && flagEstado == NORMAL)
	{
		guarda=guarda+0.2;
	}
		if(valor== 1 << IZQUIERDA && flagEstado == NORMAL)
	{
			guarda=guarda-0.2;
	}
	if(valor== 1 << CENTRO)				//CAMBIO DE ESTADO
	{
		switch (flagEstado)
		{
			case 0: sprintf(estado,"TEST    "); flagEstado=1;  break;
			case 1: sprintf(estado,"NORMAL  "); flagEstado=0;  break;
			case 2: sprintf(estado,"NORMAL  "); flagEstado=0;  break;
			default: break;
		}
	}
	
}
void EINT3_IRQHandler (void){
	
	if(LPC_GPIOINT->IO0IntStatF & ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA)))
	{
		
		LPC_GPIOINT->IO0IntEnF &= -(unsigned int)((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
		retardo_ms(200);
		LPC_GPIOINT->IO0IntEnF |= ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
	}
	if(LPC_GPIOINT->IO0IntStatR & ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA)))
	{
		
		//subida
		valor=LPC_GPIOINT->IO0IntStatR;
		LPC_GPIOINT->IO0IntEnR &= -(unsigned int)((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
		retardo_ms(200);
		flag_joystick();
		LPC_GPIOINT->IO0IntEnR |= ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
	}
	LPC_GPIOINT->IO0IntClr |= ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
}
/*---------------------------------------------------------
*							INICIALIZACION DEL JOYSTICK 									
*----------------------------------------------------------*/
void inicio_joystick(void){

	PIN_Configure(PUERTO_JOY,ABAJO,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);
	PIN_Configure(PUERTO_JOY,IZQUIERDA,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);
	PIN_Configure(PUERTO_JOY,CENTRO,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);
	PIN_Configure(PUERTO_JOY,ARRIBA,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);
	PIN_Configure(PUERTO_JOY,DERECHA,PIN_FUNC_0,PIN_PINMODE_PULLDOWN,PIN_PINMODE_NORMAL);
	
	GPIO_SetDir (PUERTO_LED1, LED_1, GPIO_DIR_OUTPUT);
	GPIO_SetDir (PUERTO_LED1, LED_2, GPIO_DIR_OUTPUT);
  GPIO_SetDir (PUERTO_LED1, LED_3, GPIO_DIR_OUTPUT);
  GPIO_SetDir (PUERTO_LED1, LED_4, GPIO_DIR_OUTPUT);
	
	GPIO_PinWrite(PUERTO_LED1, LED_1, 0);
	GPIO_PinWrite(PUERTO_LED1, LED_2, 0);
	GPIO_PinWrite(PUERTO_LED1, LED_3, 0);
	GPIO_PinWrite(PUERTO_LED1, LED_4, 0);
	
	LPC_GPIOINT->IO0IntEnR |= (1 << ABAJO) | (1 << ARRIBA) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA);
	
	NVIC_EnableIRQ(EINT3_IRQn);
	
}



