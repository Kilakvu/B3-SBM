#ifndef __HIGHBRIGHTLED_H
#define __HIGHBRIGHTLED_H

#include "stm32f4xx_hal.h"

void HighBrightLED_Init(void);
void HighBrightLED_SetDutyCycle(uint8_t duty); // 0-100

#endif
