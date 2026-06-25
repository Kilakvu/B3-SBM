#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "LCD.h"
#include "USART.h"
#include "Delay.h"
#include <stdio.h>
#include <string.h>

osThreadId_t tid_app_main;
osThreadId_t tid_usart;

const osThreadAttr_t app_main_attr = { .name = "app_main", .stack_size = 2048 };
const osThreadAttr_t usart_attr    = { .name = "usart",    .stack_size = 1024 };

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
    UART_Frame frame;

    GPIO_INIT();
    LCD_reset();
    
    // Inicializar USART y su MessageQueue
    init_USART();

    // Arrancar el hilo que escucha continuamente el puerto serie
    tid_usart = osThreadNew(USART_Thread, NULL, &usart_attr);

    lcd_clean();
    sprintf(lines[0], "     TEST USART     ");
    sprintf(lines[1], "  Esperando al PC  ");
    actualizar(lines);

    while (1) {
        // Comprobar si el hilo del USART ha ensamblado una trama completa
        if (USART_GetFrame(&frame)) {
            lcd_clean();
            
            // Mostrar en la línea 1 el código hexadecimal del comando recibido
            sprintf(lines[0], "CMD Rx: 0x%02X", frame.cmd);

            // Analizar qué comando nos han mandado para pintar la línea 2 y contestar
            switch(frame.cmd) {
                case CMD_SET_TIME:
                    sprintf(lines[1], "Set Time: %s", frame.payload);
                    USART_SendResponse(RESP_TIME_OK, (char*)frame.payload);
                    break;
                    
                case CMD_SET_LREF:
                    sprintf(lines[1], "Set Lref: %s", frame.payload);
                    USART_SendResponse(RESP_LREF_OK, (char*)frame.payload);
                    break;
                    
                case CMD_READ_MEAS:
                    sprintf(lines[1], "Leer Historico");
                    USART_SendResponse(RESP_MEASURE, "TestData Dummy");
                    break;
                    
                case CMD_CLEAR_MEAS:
                    sprintf(lines[1], "Borrar Historico");
                    USART_SendResponse(RESP_CLEAR_OK, NULL);
                    break;
                    
                default:
                    sprintf(lines[1], "CMD Desconocido");
                    break;
            }
            actualizar(lines);
        }

        osDelay(50); 
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