#include "main.h"
#include "ModeManager.h"
#include "Joystick.h"
#include "USART.h"
#include "Watchdog.h"
#include "Delay.h"
#include <stdio.h>

// Thread IDs
osThreadId_t tid_app_main;
osThreadId_t tid_joystick;
osThreadId_t tid_usart;

// Thread Attributes
const osThreadAttr_t app_main_attr = { .name = "app_main", .stack_size = 2048 };
const osThreadAttr_t joystick_attr = { .name = "joystick", .stack_size = 1024 };
const osThreadAttr_t usart_attr    = { .name = "usart",    .stack_size = 1024 };

static void SystemClock_Config(void);
static void Error_Handler(int fallo);

/**
  * @brief Thread principal de la aplicación
  */
void app_main(void *arg) {
    ModeManager_Init();

    // Give RTC time to stabilize after initialization (reduced delay)
    osDelay(200);

    tid_joystick = osThreadNew(Joystick_Thread, NULL, &joystick_attr);
    tid_usart    = osThreadNew(USART_Thread, NULL, &usart_attr);

    while (1) {
        ModeManager_Process();
        //reset_Watchdog();
        osDelay(100); // 10Hz loop
    }
}

int main(void) {
    //if (init_Watchdog() != 0) Error_Handler(2);
    
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

static void Error_Handler(int fallo) {
    while(1);
}

#ifdef RTE_CMSIS_RTOS2_RTX5
uint32_t HAL_GetTick (void) {
    if (osKernelGetState () == osKernelRunning) return (uint32_t)osKernelGetTickCount();
    for (uint32_t i = (SystemCoreClock >> 14U); i > 0U; i--) { __NOP(); }
    return 0; // Simplified for template
}
#endif