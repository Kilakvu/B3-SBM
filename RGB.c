/**
  ******************************************************************************
  * @file    RGB.c
  * @brief   Fichero de inicialización y configuración del LED RGB de la tarjeta 
  * mbed App Board.
  * Pines corregidos según hardware STM32F429:
  * - Verde: PD12 (PWM - TIM4_CH1)
  * - Rojo:  PD13 (PWM - TIM4_CH2)
  * - Azul:  PD11 (GPIO Digital Normal - ON/OFF)
  ******************************************************************************
  */

#include "RGB.h"
#include <stdio.h>

TIM_HandleTypeDef htim4;

/**
  * @brief Función de inicialización del LED RGB
  */
int initRGB (void){
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* ==========================================================
       1. CONFIGURACIÓN DEL PIN AZUL (PD11) COMO GPIO DIGITAL
       ========================================================== */
    GPIO_InitTypeDef GPIO_Azul = {0};
    GPIO_Azul.Pin = GPIO_PIN_11;
    GPIO_Azul.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Azul.Pull = GPIO_NOPULL;
    GPIO_Azul.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_Azul);

    // Apagamos el Azul inicialmente (Ánodo común -> 3.3V = Apagado)
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);

    /* ==========================================================
       2. CONFIGURACIÓN DE VERDE (PD12) Y ROJO (PD13) COMO PWM
       ========================================================== */
    GPIO_InitTypeDef GPIO_PWM = {0};
    GPIO_PWM.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_PWM.Mode = GPIO_MODE_AF_PP;
    GPIO_PWM.Pull = GPIO_NOPULL;
    GPIO_PWM.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_PWM.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_PWM);

    /* Configuración del Timer 4 */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 65535;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) return -1;

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) return -1;
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) return -1;

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) return -1;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW; // Lógica de Ánodo Común (0=Apagado, 65535=Encendido)
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    // VERDE es CH1 (PD12)
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) return -1;
    // ROJO es CH2 (PD13)
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) return -1;

    // Arrancamos los canales PWM
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

    // Inicializar apagados (0 ahora significa apagado)
    htim4.Instance->CCR1 = 0; // Verde OFF
    htim4.Instance->CCR2 = 0; // Rojo OFF

    return 0;
}

/* ==========================================================
   FUNCIONES DE CONTROL (ROJO - PWM)
   ========================================================== */
void encender_LED_rojo (int intensidad){
    htim4.Instance->CCR2 = intensidad; // Rojo es CH2
    printf("RGB: set RED intensity=%u\n", (unsigned)htim4.Instance->CCR2);
}

void apagar_LED_rojo (void){
    htim4.Instance->CCR2 = 0;
}

void intensidad_LED_rojo (int intensidad){
    htim4.Instance->CCR2 = intensidad;
}

/* ==========================================================
   FUNCIONES DE CONTROL (VERDE - PWM)
   ========================================================== */
void encender_LED_verde (int intensidad){
    htim4.Instance->CCR1 = intensidad; // Verde es CH1
    printf("RGB: set GREEN intensity=%u\n", (unsigned)htim4.Instance->CCR1);
}

void apagar_LED_verde (void){
    htim4.Instance->CCR1 = 0;
}

void intensidad_LED_verde (int intensidad){
    htim4.Instance->CCR1 = intensidad;
}

/* ==========================================================
   FUNCIONES DE CONTROL (AZUL - DIGITAL ON/OFF)
   ========================================================== */
void encender_LED_azul (int intensidad){
    // Como es digital, si le pasas cualquier intensidad mayor a 0, se enciende.
    if(intensidad > 0){
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // RESET (0V) enciende
    } else {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);   // SET (3.3V) apaga
    }
    printf("RGB: set BLUE digital state\n");
}

void apagar_LED_azul (void){
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET); // SET = Apagado
}

void intensidad_LED_azul (int intensidad){
    encender_LED_azul(intensidad);
}