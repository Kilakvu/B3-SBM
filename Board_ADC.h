#ifndef __BOARD_ADC_H
#define __BOARD_ADC_H

#include "stm32f4xx_hal.h"

int ADC1_Init(void);
uint32_t ADC_Read(uint32_t channel);

#endif
