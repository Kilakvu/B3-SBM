# Configuración del Reloj (Tic de 1 Segundo)

Para configurar un reloj que genere un "tic" exacto de 1 segundo (muy útil para el módulo HORA de tu diseño final), la documentación de la asignatura te ofrece dos alternativas: usar un Timer Virtual del RTOS (la opción obligatoria y más recomendable para el Bloque 3) o un Timer Hardware básico mediante la librería HAL.

A continuación tienes la documentación y los pasos para ambas configuraciones basándonos en tus apuntes:

---

## Opción 1: Configuración de un Timer Virtual mediante CMSIS-RTOS V2 (Recomendado)

Para proyectos que utilizan el sistema operativo, los timers virtuales permiten ejecutar una función (callback) cuando expira el tiempo configurado. Para lograr un tic de 1 segundo, debes configurar el timer en modo periódico (`osTimerPeriodic`) y asignarle un tiempo de 1000 Ticks (1000 milisegundos).

### Pasos de implementación:
1. **Definir el ID del Timer y la Callback:** Primero declaras el identificador del timer y la función que se ejecutará cada segundo.
2. **Crear el Timer:** Usando la función `osTimerNew()`, pasándole la función callback y especificando el modo periódico.
3. **Arrancar el Timer:** Usando la función `osTimerStart()` con un valor de `1000U` (1000 ms = 1 segundo).

**Ejemplo de código:**

```c
#include "cmsis_os2.h"

osTimerId_t timer_reloj_id;

// 1. Función de callback que se ejecuta cada 1 segundo
void TimerReloj_Callback (void const *arg) {
    // Aquí actualizas tus variables globales de segundos, minutos y horas [7]
}

// 2. Inicialización y arranque del timer
int Init_Reloj_Timer(void) {
    osStatus_t status;
    
    // Crear el timer en modo periódico
    timer_reloj_id = osTimerNew((osTimerFunc_t)&TimerReloj_Callback, osTimerPeriodic, NULL, NULL); [3]
    
    if (timer_reloj_id != NULL) {
        // Arrancar el timer con un periodo de 1000 ticks (1 segundo)
        status = osTimerStart(timer_reloj_id, 1000U); [3]
        if (status != osOk) {
            return -1; // Error al arrancar
        }
    }
    return 0;
}
```

---

## Opción 2: Configuración de un Timer Hardware Básico (ej. TIM6 o TIM7) con HAL

Si necesitas hacerlo a bajo nivel sin el RTOS, puedes usar los timers básicos del STM32F429 (TIM6 o TIM7), que cuentan con un contador de 16 bits y un preescalador programable.

El objetivo es generar un evento de desbordamiento (Update event) que dispare una interrupción exactamente cada 1 segundo. El tiempo de desbordamiento se calcula dividiendo la frecuencia del reloj del sistema por el valor del preescalador (PSC) y luego por el valor del registro de auto-recarga (Period/Autoreload).

### Pasos de implementación:
1. **Configurar la estructura de inicialización (`TIM_Base_InitTypeDef`):** Debes establecer los campos `Prescaler` y `Period` basándote en la frecuencia de tu bus (ej. para llegar a 1 segundo desde un reloj a 84 MHz o 168 MHz).
2. **Inicializar y Arrancar con Interrupciones:** Llama a la función `HAL_TIM_Base_Start_IT(&htim7)` para que el timer comience a contar y habilite las interrupciones.
3. **Habilitar la Interrupción en el NVIC:** Por ejemplo, usando `HAL_NVIC_EnableIRQ(TIM7_IRQn)`.
4. **Implementar la Rutina de Callback:** El driver HAL capturará la interrupción hardware y llamará automáticamente a la función `HAL_TIM_PeriodElapsedCallback`. Aquí es donde debes poner tu lógica del reloj.

**Ejemplo de estructura lógica para la Callback:**

```c
// Esta función es llamada automáticamente por HAL_