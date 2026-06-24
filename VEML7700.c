#include "VEML7700.h"

extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdev = &Driver_I2C1;

static void VEML7700_WriteReg(uint8_t reg, uint16_t val) {
    uint8_t data[3];
    data[0] = reg;
    data[1] = val & 0xFF;
    data[2] = (val >> 8) & 0xFF;
    I2Cdev->MasterTransmit(VEML7700_I2C_ADDR, data, 3, false);
    while(I2Cdev->GetStatus().busy);
}

static uint16_t VEML7700_ReadReg(uint8_t reg) {
    uint8_t data[2];
    I2Cdev->MasterTransmit(VEML7700_I2C_ADDR, &reg, 1, true);
    while(I2Cdev->GetStatus().busy);
    I2Cdev->MasterReceive(VEML7700_I2C_ADDR, data, 2, false);
    while(I2Cdev->GetStatus().busy);
    return (uint16_t)(data[1] << 8 | data[0]);
}

int VEML7700_Init(void) {
    I2Cdev->Initialize(NULL);
    I2Cdev->PowerControl(ARM_POWER_FULL);
    I2Cdev->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
    
    // Default configuration: Gain 1, Integration time 100ms, persistence 1, interrupt disable, shutdown disable
    VEML7700_WriteReg(VEML7700_ALS_CONF, 0x0000);
    return 0;
}

float VEML7700_ReadLux(void) {
    uint16_t raw = VEML7700_ReadReg(VEML7700_ALS_READ);
    // Typical resolution with Gain 1 and IT 100ms is 0.0576 lux/count
    // For simplicity, we can use a standard factor if not specified
    return (float)raw * 0.0576f;
}
