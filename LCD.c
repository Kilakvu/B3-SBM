/**
  ******************************************************************************
  * @file    LCD.c
  * @brief   Fichero de inicialización de la pantalla LCD.
  *          Configurado para SPI MSB first.
  ******************************************************************************
  */

#include "LCD.h"
#include "Arial12x12.h"
#include "Driver_SPI.h"
#include "mbedAppBoard_PINOUT.h"
#include "Delay.h"

unsigned char buffer[512];
uint8_t posicionL1=0;
uint16_t posicionL2=256;
uint16_t comienzo = 0;
int j;
char L1[128];
char L2[128];
uint8_t recoger1;
uint16_t recoger2;

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

void wr_data(unsigned char data){	
	ARM_SPI_STATUS stat;
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_A0.Port, LCD_A0.IO, GPIO_PIN_SET);	
	SPIdrv->Send(&data, 1);	
	stat = SPIdrv->GetStatus();
	while(stat.busy){
		stat = SPIdrv->GetStatus();
	}
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_SET);
}

void wr_cmd(unsigned char cmd){
	ARM_SPI_STATUS stat;
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_RESET);	
    HAL_GPIO_WritePin(LCD_A0.Port, LCD_A0.IO, GPIO_PIN_RESET);
	SPIdrv->Send(&cmd, 1);		 
	stat = SPIdrv->GetStatus();
	while(stat.busy){
		stat = SPIdrv->GetStatus();
	}
	HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_SET);
}

int LCD_reset(void){
	uint32_t status = 0;
	
	status = SPIdrv->Initialize(NULL);
	status = SPIdrv->PowerControl(ARM_POWER_FULL);
	// Corregido: Quitado ARM_SPI_MSB_LSB (que es LSB first) para usar MSB first por defecto
	status = SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_DATA_BITS(8), 1000000);

	HAL_GPIO_WritePin(LCD_RESET.Port, LCD_RESET.IO, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_RESET.Port, LCD_RESET.IO, GPIO_PIN_SET);
	HAL_Delay(10);
	
	wr_cmd(0xAE); // Display OFF
	wr_cmd(0xA2); // Bias 1/9
	wr_cmd(0xA0); // ADC Normal
	wr_cmd(0xC8); // COM scan reverse
	wr_cmd(0x21); // Resistor ratio (lowered to lighten the screen)
	wr_cmd(0x2F); // Power ON
	wr_cmd(0x40); // Start line 0
	wr_cmd(0xAF); // Display ON
	wr_cmd(0x81); // Set contrast
	wr_cmd(0x1B); // Contrast value (lowered to fix black screen)
	wr_cmd(0xA6); // Normal display
	
	lcd_clean(); // Start with a clean screen instead of black
	
	return status;
}

void copy_to_lcd(void){
    int i;
    for(int p=0; p<4; p++) {
        wr_cmd(0x00);      
        wr_cmd(0x10);      
        wr_cmd(0xB0 | p);      
        for(i=0; i<128; i++){
            wr_data(buffer[i + p*128]);
        }
    }
}

void lcd_clean(void){
	for(int i=0; i<512; i++){
		buffer[i]=0;
	}
	copy_to_lcd();
}

int GetStringWidth(const char* str) {
    int width = 0;
    for (int i = 0; i < strlen(str); i++) {
        uint16_t idx = 25 * ((uint8_t)str[i] - ' ');
        width += Arial12x12[idx];
    }
    return width;
}

int EscribeLetra_L1 (uint8_t letra){
	uint8_t i, valor1, valor2;
	if(posicionL1 <= 127){
		comienzo = 25 * (letra - ' ');
		for (i=0; i<12; i++){
			if (i+posicionL1 >= 128) break;
			valor1 = Arial12x12[comienzo+i*2+1];
			valor2 = Arial12x12[comienzo+i*2+2];
			buffer[i+posicionL1] = valor1;
			buffer[i+128+posicionL1] = valor2;
		}
		posicionL1 = posicionL1 + Arial12x12[comienzo];
	}
	return 0;	
}

int EscribeLetra_L2 (uint16_t letra){
	uint8_t i, valor1, valor2;
	if(posicionL2 <= 383){
		comienzo = 25 * (letra - ' ');
		for (i=0; i<12; i++){
			if (i+(posicionL2-256) >= 128) break;
			valor1 = Arial12x12[comienzo+i*2+1];
			valor2 = Arial12x12[comienzo+i*2+2];
			buffer[i+posicionL2] = valor1;
			buffer[i+128+posicionL2] = valor2;
		}
		posicionL2 = posicionL2 + Arial12x12[comienzo];
	}
	return 0;	
}

void LCD_DrawBar(uint8_t x, uint8_t height) {
    if (x > 124) x = 124;
    if (height > 32) height = 32;

    for (int col = x; col < x + 4; col++) {
        for (int p = 0; p < 4; p++) {
            uint8_t page_pixels = 0;
            int page_base_row = (3 - p) * 8; 
            for (int r = 0; r < 8; r++) {
                if ((page_base_row + r) < height) {
                    page_pixels |= (1 << (7 - r));
                }
            }
            buffer[col + p * 128] = page_pixels;
        }
    }
}

void actualizar(char lcd_text[2][20+1]){
	int w1 = GetStringWidth(lcd_text[0]);
	int w2 = GetStringWidth(lcd_text[1]);
	
	posicionL1 = (w1 < 128) ? (128 - w1) / 2 : 0;
	posicionL2 = 256 + ((w2 < 128) ? (128 - w2) / 2 : 0);

	for (j=0; j<strlen(lcd_text[0]); j++){
		EscribeLetra_L1((uint8_t)lcd_text[0][j]);
	}
	for (j=0; j<strlen(lcd_text[1]); j++){
		EscribeLetra_L2((uint16_t)lcd_text[1][j]);
	}
	copy_to_lcd();
}

void pant_neg (void){
	for(int i=0; i<512; i++){
		buffer[i]=0xff;
	}
	copy_to_lcd();
}

void GPIO_INIT(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(LCD_RESET.Port, LCD_RESET.IO, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_A0.Port, LCD_A0.IO, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_CS_N.Port, LCD_CS_N.IO, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = LCD_RESET.IO;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET.Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LCD_A0.IO;
  HAL_GPIO_Init(LCD_A0.Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LCD_CS_N.IO;
  HAL_GPIO_Init(LCD_CS_N.Port, &GPIO_InitStruct);
}
