#ifndef __PWM_H 
#define __PWM_H
#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"



/*************************************************************************************************
                           PWM Config Bit Positions
*************************************************************************************************/
#define PWM_1   0
#define PWM_2   2
#define PWM_3   4
#define PWM_4   6

/***************************************************************************************************
                             Function prototypes
***************************************************************************************************/
void PWM_vInit(void);	//Conf PWM
void Activar_Alarma(void); //Activar Speaker
void Desactivar_Alarma(void);	//Desactivar Skeaker
void RGB_AZUL(void);	//ON AZUL
void RGB_ROJO(void);	//ON ROJO
void RGB_VERDE(void); //ON VERDE
void initTimer0(void); //INCIO TIMER PWM
/**************************************************************************************************/

#endif /* end __PWM_H */
