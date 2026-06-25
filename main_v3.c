#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "LCD.h"
#include "VEML7700.h"
#include "Delay.h"
#include <stdio.h>

osThreadId_t tid_app_main;
const osThreadAttr_t app_main_attr = { .name = "app_main", .stack_size = 2048 };

static void SystemClock_Config(void);

#ifdef RTE_CMSIS_RTOS2_RTX5
uint32_t HAL_GetTick(void) {
    if (osKernelGetState() == osKernelRunning) return (uint32_t)osKernelGetTickCount();
    for (uint32_t i = (SystemCoreClock >> 14U); i > 0U; i--) { __NOP(); }
    return 0;
}
#endif

void app_main(void *arg) {
    char lines[2][21];
    
    // Inicializar hardware básico y periféricos
    GPIO_INIT();
    LCD_reset();
    
    // Inicializar el sensor I2C
    VEML7700_Init();

    // Mensaje de bienvenida
    lcd_clean();
    sprintf(lines[0], " Test VEML7700 ");
    sprintf(lines[1], " Iniciando...  ");
    actualizar(lines);
    
    // Dar medio segundo al sensor para realizar su primera integración de luz
    osDelay(500); 

    while (1) {
        // Leer el valor en luxes
        float lux = VEML7700_ReadLux();
        
        // Actualizar LCD
        lcd_clean();
        sprintf(lines[0], "Sensor de Luz:");
        sprintf(lines[1], "Lmed: %06.2f lx", lux);
        actualizar(lines);
        
        // Refresco a 4Hz (250ms). Es una velocidad cómoda para poder
        // leer los números sin que bailen demasiado rápido en la pantalla.
        osDelay(250); 
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    SystemCoreClockUpdate();
    Init_Delay(180, 4);

    osKernelInitialize();
    tid_app_main = osThreadNew(app_main, NULL, &app_main_attr);
    osKernelStart();

    while (1);
}

static void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    HAL_PWREx_EnableOverDrive();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}