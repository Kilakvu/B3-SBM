#ifndef __VEML7700_H
#define __VEML7700_H

#include "stm32f4xx_hal.h"
#include "Driver_I2C.h"

// VEML7700 I2C Address
#define VEML7700_I2C_ADDR 0x10

// Registers
#define VEML7700_ALS_CONF       0x00
#define VEML7700_ALS_WH         0x01
#define VEML7700_ALS_WL         0x02
#define VEML7700_PS_CONF        0x03
#define VEML7700_ALS_READ       0x04
#define VEML7700_WHITE_READ     0x05
#define VEML7700_ALS_INT        0x06

int VEML7700_Init(void);
float VEML7700_ReadLux(void);

#endif
