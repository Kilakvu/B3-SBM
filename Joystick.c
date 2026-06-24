#include "Joystick.h"

static osThreadId_t TID_Joystick = NULL;
static uint32_t joystick_events = 0;
static osEventFlagsId_t joy_event_flags;

void Joystick_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure pins as input with interrupt on both edges (to detect press and release)
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = JOY_DOWN_PIN | JOY_LEFT_PIN | JOY_CENTER_PIN;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = JOY_UP_PIN | JOY_RIGHT_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Enable IRQs
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    joy_event_flags = osEventFlagsNew(NULL);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // Basic debouncing would be needed, but for now we signal the thread
    osEventFlagsSet(joy_event_flags, GPIO_Pin);
}

// In stm32f4xx_it.c I will need to call HAL_GPIO_EXTI_IRQHandler for pins 10-15
// For now, let's assume we have a thread that monitors the buttons or handles the logic.

// Actually, a better approach for short/long press:
// ISR signals a thread. Thread waits, then debounces and measures time.

void Joystick_Thread(void *argument) {
    uint32_t flags;
    uint32_t start_time[16] = {0}; // To store tick when pressed
    uint8_t pressed[16] = {0};

    while (1) {
        flags = osEventFlagsWait(joy_event_flags, 0xFFFF, osFlagsWaitAny, osWaitForever);
        
        uint32_t current_time = osKernelGetTickCount();
        
        // Iterate through possible pins (10-15)
        for (int i = 10; i <= 15; i++) {
            uint16_t pin = (1 << i);
            if (flags & pin) {
                // Check current state
                GPIO_PinState state;
                if (i == 10 || i == 11) state = HAL_GPIO_ReadPin(GPIOB, pin);
                else state = HAL_GPIO_ReadPin(GPIOE, pin);

                if (state == GPIO_PIN_SET) {
                    if (!pressed[i]) {
                        pressed[i] = 1;
                        start_time[i] = current_time;
                    }
                } else {
                    if (pressed[i]) {
                        pressed[i] = 0;
                        uint32_t duration = current_time - start_time[i];
                        
                        uint32_t event = 0;
                        switch(pin) {
                            case JOY_UP_PIN:     if (i==10) event = JOY_EVENT_UP; break; // PB10
                            case JOY_RIGHT_PIN:  if (i==11) event = JOY_EVENT_RIGHT; break; // PB11
                            case JOY_DOWN_PIN:   event = JOY_EVENT_DOWN; break;
                            case JOY_LEFT_PIN:   event = JOY_EVENT_LEFT; break;
                            case JOY_CENTER_PIN: event = JOY_EVENT_CENTER; break;
                        }

                        if (event) {
                            if (duration > 1000) {
                                joystick_events |= (event | JOY_EVENT_LONG);
                            } else if (duration > 50) { // Debounce
                                joystick_events |= event;
                            }
                        }
                    }
                }
            }
        }
    }
}

uint32_t Joystick_GetEvents(void) {
    uint32_t events = joystick_events;
    joystick_events = 0;
    return events;
}
