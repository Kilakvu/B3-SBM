/*----------------------------------------------------------------------------	
*BLOQUE 3. Controlador de las condiciones ambientales de un recinto
ALUMNO: RICARDO GÓMEZ MATEOS
VERSION: FINAL
FECHA: 18/12/2020

*-----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/

#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "joystick.h"
#include "pwm.h"
#include "lcd.h"
#include "escritura.h" 
extern int Init_Thread (void);
extern void inicio_joystick(void);
/*
 * main: initialize and start the system
 */
int main (void) {
  osKernelInitialize ();                    // initialize CMSIS-RTOS
	
	inicio_joystick();												//Inicializacion del Joystick
	EINT3_IRQHandler();												//Habilitacion de interrupciones por EINT3
	
	initTimer0();															//Timer del PWM
	
	PWM_vInit();															//Inicializacion del PWM1.X
	
	Init_Thread();														//Sistema Thread init
 

  osKernelStart ();                         // start thread execution 
}
