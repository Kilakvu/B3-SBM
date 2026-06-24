#include "escritura.h"
#include "retardo.h"
#include "PIN_LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "Driver_SPI.h"

#define gpioPORT_SSP1	0
#define pin_nRESET_SP1	8
#define pin_A0_SSP1		6
#define CS 		18

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI *SPIDrv = &Driver_SPI1;
extern uint8_t buffer[512];



void wr_cmd(uint8_t cmd)
{
	GPIO_PinWrite (gpioPORT_SSP1, CS, 0);
	GPIO_PinWrite (gpioPORT_SSP1, pin_A0_SSP1,0);
	SPIDrv ->Send(&cmd,sizeof(cmd));
	GPIO_PinWrite (gpioPORT_SSP1,CS,1);
}

void wr_dat(uint8_t dat)
{
	GPIO_PinWrite (gpioPORT_SSP1, CS, 0);
	GPIO_PinWrite (gpioPORT_SSP1, pin_A0_SSP1,1);
	SPIDrv ->Send(&dat,sizeof(dat));
	GPIO_PinWrite (gpioPORT_SSP1,CS,1);
}

void copy_to_lcd_linea_1(void)
{
    int i;
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB0);      // Página 0
    
    for(i=0;i<128;i++){
        wr_dat(buffer[i]);
			
        }
  
     
    wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0
    wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0
    wr_cmd(0xB1);      // Página 1
     
    for(i=128;i<256;i++){
        wr_dat(buffer[i]);
        } 
}

void copy_to_lcd_linea_2(void)
{
	int i;
	  wr_cmd(0x00);       
    wr_cmd(0x10);      
    wr_cmd(0xB2);      //Página 2
    for(i=256;i<384;i++){
        wr_dat(buffer[i]);
        }
    
    wr_cmd(0x00);       
    wr_cmd(0x10);       
    wr_cmd(0xB3);      // Pagina 3
     
     
    for(i=384;i<512;i++){
        wr_dat(buffer[i]);
        }
}

void init (void){
	
	
	SPIDrv->Initialize(NULL);
	SPIDrv -> PowerControl(ARM_POWER_FULL);
	SPIDrv ->Control (ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 |ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8),20000000);
	
  
  //ARM_SPI_SS_MASTER_HW_OUTPUT |
	//PIN_Configure(gpioPORT_SSP1, pin_nRESET_SP1, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
  
  //PIN_Configure(gpioPORT_SSP1, pin_A0_SSP1, PIN_FUNC_0, PIN_PINMODE_PULLUP, PIN_PINMODE_NORMAL);
	
	GPIO_SetDir (gpioPORT_SSP1, pin_nRESET_SP1, GPIO_DIR_OUTPUT);
	GPIO_SetDir(gpioPORT_SSP1, pin_A0_SSP1, GPIO_DIR_OUTPUT);
	GPIO_SetDir(gpioPORT_SSP1,CS, GPIO_DIR_OUTPUT);
	
	GPIO_PinWrite(gpioPORT_SSP1,pin_A0_SSP1,0);
	GPIO_PinWrite(gpioPORT_SSP1,pin_nRESET_SP1,1);
	GPIO_PinWrite(gpioPORT_SSP1,CS,1);
	GPIO_PinWrite(gpioPORT_SSP1,pin_nRESET_SP1,0);
	
	retardo_1us();
	
	GPIO_PinWrite(gpioPORT_SSP1,pin_nRESET_SP1,1);
	
	retardo_1Ms();
	

}

void lcd_reset(void){
 
wr_cmd(0xAE);	//DISPLAY EN OFF	
wr_cmd(0xA2); //Fija el valor de la relación de la tensión de polarización del LCD a 1/9 
wr_cmd(0xA0); //El direccionamiento de la RAM de datos del display es la normal
wr_cmd(0xC8); //El scan en las salidas COM es el normal
wr_cmd(0x22); //Fija la relación de resistencias interna a 2
wr_cmd(0x2F);   //Power on
wr_cmd(0x40);    //Display empieza en la línea 0
wr_cmd(0xAF);    //Display ON
wr_cmd(0x81);    //Contraste
wr_cmd(0x17);    //Valor Contraste
wr_cmd(0xA4);   //Display all points normal
wr_cmd(0xA6);   //LCD Display normal 



	
}

