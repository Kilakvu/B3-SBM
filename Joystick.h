#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

// Joystick Pins
#define JOY_DOWN_PIN    GPIO_PIN_12
#define JOY_DOWN_PORT   GPIOE
#define JOY_LEFT_PIN    GPIO_PIN_14
#define JOY_LEFT_PORT   GPIOE
#define JOY_CENTER_PIN  GPIO_PIN_15
#define JOY_CENTER_PORT GPIOE
#define JOY_UP_PIN      GPIO_PIN_10
#define JOY_UP_PORT     GPIOB
#define JOY_RIGHT_PIN   GPIO_PIN_11
#define JOY_RIGHT_PORT  GPIOB

// Events
#define JOY_EVENT_UP      (1 << 0)
#define JOY_EVENT_DOWN    (1 << 1)
#define JOY_EVENT_LEFT    (1 << 2)
#define JOY_EVENT_RIGHT   (1 << 3)
#define JOY_EVENT_CENTER  (1 << 4)
#define JOY_EVENT_LONG    (1 << 5) // Flag for long press

void Joystick_Init(void);
uint32_t Joystick_GetEvents(void);
void Joystick_Thread(void *argument);

#endif
