#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "USART.h"
#include "Delay.h"
#include <string.h>

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

static void ProcessFrame(UART_Frame *frame) {
    switch (frame->cmd) {
        case CMD_SET_TIME:
            USART_SendResponse(RESP_TIME_OK, (const char *)frame->payload);
            break;
        case CMD_SET_LREF:
            USART_SendResponse(RESP_LREF_OK, (const char *)frame->payload);
            break;
        case CMD_READ_MEAS:
            USART_SendResponse(RESP_MEASURE, "OK");
            break;
        case CMD_CLEAR_MEAS:
            USART_SendResponse(RESP_CLEAR_OK, NULL);
            break;
        default: {
            char echo[64];
            int n = snprintf(echo, sizeof(echo), "ECO: cmd=0x%02X len=%d", frame->cmd, frame->len);
            if (frame->len > 4) {
                frame->payload[frame->len - 4] = '\0';
                snprintf(echo + n, sizeof(echo) - n, " payload=%s", frame->payload);
            }
            USART_SendResponse(frame->cmd, echo);
            break;
        }
    }
}

void app_main(void *arg) {
    UART_Frame frame;

    init_USART();

    printf("\r\n=== Test USART Eco ===\r\n");
    printf("Comandos: SET_TIME(0x20), SET_LREF(0x25), READ_MEAS(0x55), CLEAR_MEAS(0x60)\r\n");
    printf("Cualquier otro comando hara eco\r\n");

    while (1) {
        if (USART_GetFrame(&frame)) {
            ProcessFrame(&frame);
        }
        osDelay(10);
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
