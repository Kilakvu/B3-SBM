#ifndef _define
#define _define

#include "lcd.h"  						
#include "escritura.h"
#include <stdio.h>
#include <string.h>
#include "Driver_USART.h"
#include "cmsis_os.h"                   /* ARM::CMSIS:RTOS:Keil RTX */
#include <stdio.h>
#include <string.h>
#include "retardo.h"
#include "pwm.h"
#include "joystick.h"

/*************************************************************************************************
                           IC2 CONFIGURACION
*************************************************************************************************/
#define MMA7660_I2C_ADDR 0x4C
#define REG_MODE  0x07
#define REG_TILT  0x03
#define LM75B_I2C_ADDR	0x48
#define LM75_REG_TEMP (0x00) // Temperature Register
#define LM75_REG_CONF (0x01) // Configuration Register
#define LM75_REG_TOS (0x03) // TOS Register
#define LM75_REG_THYST (0x02) // THYST Register
/*************************************************************************************************
                           ESTADOS PARA TRANSITAR
*************************************************************************************************/
#define NORMAL 0
#define TEST 1
#define DESARMADO 2
/*************************************************************************************************
                           VARIABLES MAIN
*************************************************************************************************/
char mensaje[30];			//cadena chart para el LCD
char mensajeUART[34];	//cadena de char para el USART
char letraTest[4]="RLF";		//Letra escrita en el lcd grande y posicionada
float temp=0;
float guarda=2.0;		//Valor de guarda
float tmepRef=25.0;	//Valor de temp Ref.
uint16_t i16;
char estado[10]="NORMAL";		//ESTADO
int flagEstado=0;						//FLAG AUX PARA EL ESTADO
uint8_t data_read[2];				//DATOS PARA LA CONFIGURACION I2C
uint8_t data_write[2];
uint8_t PoLa;								//POSICION DE LA TARJETA
uint8_t hour=0, min=0, sec=0;		//HH:MM:SS
uint8_t horaDes, minDes, secDes;	//HH:MM:SS del momento desarmado
int flag_timer2=0;


#endif
