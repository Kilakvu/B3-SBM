#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "LCD.h"
#include "Joystick.h"
#include "RGB.h"
#include "HighBrightLED.h"
#include "Board_ADC.h"
#include "Delay.h"
#include <stdio.h>

// Identificadores de los hilos
osThreadId_t tid_app_main;
osThreadId_t tid_joystick; 

const osThreadAttr_t app_main_attr = { .name = "app_main", .stack_size = 2048 };
const osThreadAttr_t joystick_attr = { .name = "joystick", .stack_size = 1024 }; 

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
    int red_state = 0;
    int blue_state = 0;
    
    // Inicializar todos los periféricos del test
    Joystick_Init();
    GPIO_INIT();
    LCD_reset();
    ADC1_Init();
    initRGB();
    HighBrightLED_Init();

    // Arrancar el hilo del joystick para capturar eventos
    tid_joystick = osThreadNew(Joystick_Thread, NULL, &joystick_attr);

    while (1) {
        // 1. GESTIÓN DEL JOYSTICK (LEDs Rojo y Azul)
        uint32_t events = Joystick_GetEvents();

        if (events & JOY_EVENT_UP) {
            red_state = !red_state; // Alternar estado
            if (red_state) encender_LED_rojo(65535);
            else apagar_LED_rojo();
        }
        
        if (events & JOY_EVENT_DOWN) {
            blue_state = !blue_state; // Alternar estado
            if (blue_state) encender_LED_azul(1);
            else apagar_LED_azul();
        }

        // 2. GESTIÓN DEL ADC (Potenciómetro 2 -> LED Verde y HighBright)
        uint16_t adc_val = ADC_Read(ADC_CHANNEL_10); 
        
        // Mapear el valor del ADC (0-4095) a los rangos de los LEDs
        uint8_t pwm_100 = (adc_val * 100) / 4095;         // Rango 0-100% para HighBright
        uint16_t pwm_16b = (adc_val * 65535) / 4095;      // Rango 0-65535 para RGB Verde

        // Aplicar las intensidades
        HighBrightLED_SetDutyCycle(pwm_100);
        encender_LED_verde(pwm_16b);

        // 3. ACTUALIZAR PANTALLA LCD
        lcd_clean();
        sprintf(lines[0], "HB:%03d%% Vrd:%03d%%", pwm_100, pwm_100);
        sprintf(lines[1], "Roj:%s Azl:%s", red_state ? "ON " : "OFF", blue_state ? "ON " : "OFF");
        actualizar(lines);

        // Refresco a 10Hz
        osDelay(100); 
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