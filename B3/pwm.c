#include "lpc17xx.h"
#include "pwm.h"
#include "GPIO_LPC17xx.h"



void PWM_vInit(void)
{
  //init PWM
  LPC_SC->PCONP |= (1 << 6);            //PWM on
 LPC_PINCON->PINSEL4 &=~3;            //reset
 LPC_PINCON->PINSEL4 = (1<<0)|(1<<2)|(1<<4)|(1<<6);            //set PWM1.1 at P2.0
 LPC_PWM1->TCR = 2;                    //counter reset
 LPC_PWM1->PR = 24;                    //clock /4 / prescaler (= PR +1) = 1 µs
 LPC_PWM1->MCR = (1<<1);            //reset on MR0
 LPC_PWM1->MR0 = 255;                //set PWM cycle 1khz
 LPC_PWM1->MR1 = 254;                //set duty
 LPC_PWM1->MR2 = 254;
 LPC_PWM1->MR3 = 254;
 LPC_PWM1->MR4 = 254;
	
 LPC_PWM1->LER = (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4);        //latch MR0 & MR1
 LPC_PWM1->PCR = (1<<9)|(1<<10)|(1<<11)|(1<<12);            //PWM1 output enable
 LPC_PWM1->TCR = (1<<0)|(1<<3);        //counter enable, PWM enable
}
void Activar_Alarma(void)
{
	LPC_PWM1->MR1 = 100; //Update MR1 with new value
	LPC_PWM1->LER = (1<<1); //Load the MR1 new value at start of next cycle
}
void Desactivar_Alarma(void)
{
	LPC_PWM1->MR1 = 0; //Update MR1 with new value
	LPC_PWM1->LER = (1<<1); //Load the MR1 new value at start of next cycle
}
void RGB_AZUL(void)
{
	
	LPC_PWM1->MR2 = 0; //Update MR1 with new value
	LPC_PWM1->MR3 = 255;
	LPC_PWM1->MR4 = 255;
	LPC_PWM1->LER |= (1<<2); //Load the MR1 new value at start of next cycle
}
void RGB_VERDE(void)
{
	
	LPC_PWM1->MR2 = 255; //AZUL
	LPC_PWM1->MR3 = 0;		//VERDE
	LPC_PWM1->MR4 = 255;	//ROJO
	LPC_PWM1->LER |= (1<<3); //Load the MR1 new value at start of next cycle
}
void RGB_ROJO(void)
{
	
	LPC_PWM1->MR2 = 255; //Update MR1 with new value
	LPC_PWM1->MR3 = 255;
	LPC_PWM1->MR4 = 0;
	LPC_PWM1->LER |= (1<<4); //Load the MR1 new value at start of next cycle
}

void initTimer0(void) //To setup Timer0 used delayMS() function
{
	/*Assuming that PLL0 has been setup with CCLK = 100Mhz and PCLK = 25Mhz.*/

	LPC_TIM0->CTCR = 0x0;
	LPC_TIM0->PR = 25000-1; //Increment TC at every 24999+1 clock cycles
	//25000 clock cycles @25Mhz = 1 mS

	LPC_TIM0->TCR = 0x02; //Reset Timer
}
