# Integración y desarrollo de una aplicación: Sistema de Control de Iluminación Ambiental

**Departamento de Ingeniería Telemática y Electrónica**
**Universidad Politécnica de Madrid | 2025-26**
**Sistemas Basados en Microprocesador**

* **Alumno A:** …………………………..
* **Alumno B:** …………………………..
* **Puesto Nº:** x

---

## 1. Objetivos de la PRÁCTICA

### 1.1 Resumen de los objetivos de la práctica realizada

- **Desarrollar un sistema embebido de control de iluminación ambiental** en base a un microcontrolador STM32F429ZI que permita la gestión de diferentes modos de operación.
- **Implementar un gestor de estados** que coordine la transición entre cuatro modos de funcionamiento: REPOSO, MANUAL, AUTO y PROGRAMACIÓN.
- **Integrar sensores y actuadores** incluyendo un sensor de luz ambiental (VEML7700) y LEDs RGB para retroalimentación visual.
- **Diseñar una interfaz de usuario** mediante joystick, pantalla LCD y comunicación serie UART para monitorización y calibración del sistema.
- **Implementar algoritmos de control** que permitan tanto operación manual como automática con regulación PWM de luminosidad.
- **Aplicar el uso de RTOS (CMSIS-RTOS2)** para gestionar múltiples tareas concurrentes de forma eficiente.

### 1.2 Acrónimos utilizados

| Acrónimo | Descripción |
| :--- | :--- |
| **STM32F429ZI** | Microcontrolador ARM Cortex-M4 de STMicroelectronics |
| **UART** | Universal Asynchronous Receiver Transmitter |
| **PWM** | Pulse Width Modulation (Modulación por Ancho de Pulso) |
| **I2C** | Inter-Integrated Circuit (Bus de comunicación serie) |
| **ADC** | Conversor Analógico-Digital |
| **RTC** | Real-Time Clock (Reloj de Tiempo Real) |
| **RTOS** | Real-Time Operating System |
| **LCD** | Liquid Crystal Display |
| **RGB** | Red-Green-Blue (Rojo-Verde-Azul) |
| **HAL** | Hardware Abstraction Layer |
| **GPIO** | General Purpose Input/Output |
| **VEML7700** | Sensor de luz ambiental digital |
| **Lux** | Unidad de iluminancia |
| **Lmed** | Luminosidad medida por el sensor |
| **Lref** | Luminosidad de referencia configurada |
| **SBM** | (Título del proyecto) |

### 1.3 Tiempo empleado en la realización de la práctica

El tiempo total empleado en la realización de la práctica ha sido de aproximadamente **45-50 horas**, distribuidas en:
- Análisis de requisitos y diseño: 5 horas
- Configuración de hardware y periféricos: 8 horas
- Implementación de módulos individuales: 20 horas
- Integración del sistema: 10 horas
- Pruebas, depuración y optimización: 12 horas

### 1.4 Bibliografía utilizada

- **STM32F429ZI Reference Manual (RM0090)** - STMicroelectronics
- **NUCLEO-STM32F429ZI User Manual** - STMicroelectronics
- **VEML7700 Datasheet** - Vishay Semiconductors
- **ARM CMSIS-RTOS2 Documentation** - Arm Ltd.
- **Keil MDK-ARM User's Manual** - Arm Ltd.
- **mbed AppBoard Schematic and User Guide** - ARM mbed
- **C Programming Language Reference**

### 1.5 Autoevaluación

**Objetivos de aprendizaje alcanzados:**
- ✓ Configuración avanzada de periféricos del STM32F429ZI (UART, I2C, ADC, PWM, RTC)
- ✓ Programación en C con gestión de memoria y optimización
- ✓ Uso de RTOS para multitarea y sincronización
- ✓ Programación de drivers para sensores (I2C)
- ✓ Diseño e implementación de máquinas de estados complejas
- ✓ Debugging y testing de sistemas embebidos
- ✓ Comunicación serie con protocolo estructurado

**Dificultades encontradas y resueltas:**
- Sincronización de threads con múltiples periféricos: Resuelta con uso de eventos de Joystick y buffer circular.
- Calibración del sensor VEML7700: Se implementó lectura repetida para obtener valores estables.
- Interfaz de usuario en LCD limitado: Diseño optimizado de visualización de información en dos líneas.

---

## 2. RECURSOS UTILIZADOS DEL MICROCONTROLADOR

### 2.1 Diagrama de bloques hardware del sistema

```
┌─────────────────────────────────────────────────────────────┐
│           STM32F429ZI Microcontroller (ARM Cortex-M4)       │
│                  @ 180 MHz (Clock Config)                    │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐    │
│  │ UART (PA2)  │     │ I2C1 (PB8)  │     │ ADC1/3 (PA4)│    │
│  │ (NUCLEO)    │     │ (VEML7700)  │     │ (Joystick)  │    │
│  └─────────────┘     └─────────────┘     └─────────────┘    │
│         │                   │                    │             │
│  ┌──────▼──────────────────▼────────────────────▼─────┐      │
│  │         GPIO & Periféricos Digitales               │      │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────────┐      │      │
│  │  │   TIM4   │  │   RTC    │  │   PWM (TIM1) │      │      │
│  │  │(Encoder) │  │  Clock   │  │  HighBright  │      │      │
│  │  └──────────┘  └──────────┘  └──────────────┘      │      │
│  └────────────────────────────────────────────────────┘      │
│                                                               │
└─────────────────────────────────────────────────────────────┘
        │             │              │             │
        ▼             ▼              ▼             ▼
   ┌────────┐   ┌──────────┐  ┌────────────┐  ┌────────┐
   │ USART  │   │ VEML7700 │  │ LED RGB    │  │Joystick│
   │ (PC)   │   │(Luz)     │  │(mbed Board)│  │5 botones│
   └────────┘   └──────────┘  └────────────┘  └────────┘
        │             │              │             │
   ┌────────┐   ┌──────────┐  ┌────────────┐  ┌────────┐
   │Terminal│   │I2C Sensor│  │R:PE7,G:PE8 │  │CENTER  │
   │Serial  │   │@0x10     │  │B:PE9       │  │UP/DOWN │
   └────────┘   └──────────┘  │HighBright: │  │L/RIGHT │
                               │PE11        │  └────────┘
                               └────────────┘
```

### 2.2 Cálculos realizados y justificación de la solución adoptada

#### **Configuración de Reloj (SystemClock_Config)**
```
Entrada: HSI = 16 MHz (oscilador interno)
PLL CONFIG: PLLM=8, PLLN=180, PLLP=2, PLLQ=4
Cálculo: f_vco = 16 MHz × 180 / 8 = 360 MHz
         f_sys = 360 MHz / 2 = 180 MHz ✓

Divisores de Bus:
- HCLK (AHB):   180 MHz / 1 = 180 MHz
- APB1 (Low):   180 MHz / 4 = 45 MHz  (Timers × 2 = 90 MHz)
- APB2 (Fast):  180 MHz / 2 = 90 MHz  (Timers × 2 = 180 MHz)
```

#### **Configuración ADC (Joystick)**
```
Canal ADC1:
- ADC_CHANNEL_3  (PA3): Referencia de Luz (Lref)
- ADC_CHANNEL_10 (PC0): PWM duty cycle control
- Resolución: 12 bits (0-4095)
- Rango de entrada: 0-3.3V

Conversión ADC a valores físicos:
- Lref_ADC: 100 + (ADC_Read(3) × 3900 / 4095) lux
  Rango: 100-4000 lux (barrido completo)
- PWM_ADC:  5 + (ADC_Read(10) × 90 / 4095) %
  Rango: 5%-95% duty cycle
```

#### **Configuración PWM (LED de Alta Intensidad)**
```
Timer: TIM1 (APB2 timer clock @ 180 MHz)
Prescaler: 180 - 1 (divide por 180 → timer clock ≈ 1 MHz)
Auto-Reload (Period): 1000 - 1
Frecuencia PWM: 1 MHz / 1000 = 1 kHz (configuración usada en `HighBrightLED.c`)

Cálculo:
- 100% duty = ARR (1000) counts
- Resolución: 1 count = 0.1% (aprox.) → 1000 steps para 0-100%
```

#### **Configuración RTC**
```
Fuente: LSE (External 32.768 kHz crystal)
Prescaler: 32768 - 1 (1-second counter)
Rango de tiempo: 24-hour format
Hora inicial: 09:50:00 (según especificación CA1)
```

#### **Configuración I2C (VEML7700)**
```
Bus I2C1: PB8 (SCL), PB9 (SDA)
Dirección I2C: 0x10 (VEML7700)
Velocidad: 100 kHz (modo estándar)
Registros utilizados:
- 0x00 (ALS_CONF): Configuración de resolución
- 0x04 (ALS_READ): Lectura de valor de luz
- 0x05 (WHITE_READ): Lectura de componente blanco

Resolución configurada: 0.0036 lux/step
Rango: 0-120000 lux
```

#### **Regulación PWM (Modo AUTO) - Control Proporcional**
```
Error = Lref - Lmed

Algoritmo:
IF error > 10 lux THEN pwa += 1 (aumentar intensidad)
IF error < -10 lux THEN pwm -= 1 (reducir intensidad)
ELSE mantener constante

Rango de PWM: [5%, 95%]
Resolución: 1% por step
Tiempo de actualización: 100 ms (parte de ModeManager_Process)

Justificación: Control simple pero efectivo para evitar oscilaciones.
La banda muerta de ±10 lux previene jitter alrededor del setpoint.
```

#### **Barrido de LED Verde en Reposo**
```
Periodo total: 4 segundos (2s up, 2s down)
Intensidad: rango 10%-90% (80% de amplitud)

Cálculo de interpolación lineal:
elapsed = tick_actual % 4000 ms

Si elapsed < 2000 ms (fase de ascenso):
  intensity = 0.1 + 0.8 × (elapsed / 2000)
  
Si elapsed ≥ 2000 ms (fase de descenso):
  intensity = 0.9 - 0.8 × ((elapsed - 2000) / 2000)

Valor PWM = intensity × 65535 (16 bits)
```

---

## 3. SOFTWARE

### 3.1 Descripción de cada uno de los módulos del sistema

#### **ModeManager.c / ModeManager.h**
**Propósito:** Gestor central de máquina de estados y orquestador de todas las funcionalidades.

**Funciones principales:**
- `ModeManager_Init()`: Inicializa todos los periféricos y módulos
- `ModeManager_Process()`: Loop principal que procesa eventos y cambios de modo

**Máquina de Estados:**
```
    ┌────────────┐
    │  REPOSO    │ (Inicialización por defecto)
    │ LED verde  │ (barrido PWM 10-90%, 2s/ciclo)
    └─┬──────┬───┘
      │      │
      │ LONG │
      │CENTER│
      │      │
    ┌─▼──┐ ┌─▼──┐
    │    │ │    │
    │MAN └─┼─▶AUTO──┐
    │UAL   │      │ LONG
    │      │      │CENTER
    │  LONG│      │
    │  CENTER      │
    │      │      │
    │  ┌─▶PROGRAM◄─┘
    │  │      │
    │  └──────┘
    └────────────┘
```

**Variables compartidas:**
- `current_mode`: Modo operativo actual
- `l_med`: Luminosidad medida por VEML7700 (en lux)
- `l_ref`: Luminosidad de referencia (100-4000 lux)
- `pwm_val`: Valor de duty cycle (5-95%)
- `buffer_circular[20][64]`: Buffer circular para log de medidas

#### **Joystick.c / Joystick.h**
**Propósito:** Gestor de entrada del joystick con detección de pulsación larga.

**Periféricos:**
- GPIO: PE12 (DOWN), PE14 (LEFT), PE15 (CENTER), PB10 (UP), PB11 (RIGHT)
- Debouncing: 20 ms entre lecturas

**Eventos generados:**
- `JOY_EVENT_UP, DOWN, LEFT, RIGHT, CENTER`: eventos de pulsación cortaBounce
- `JOY_EVENT_LONG`: flag adicional cuando la pulsación excede 1 segundo

**Thread:** Se ejecuta cada 20 ms, actualizando estado de botones y detectando cambios.

#### **VEML7700.c / VEML7700.h**
**Propósito:** Driver para sensor de luz ambiental vía I2C.

**Registro de configuración (0x00):**
- Resolución: 0.0036 lux/step
- Modo de medición: Continuo
- Rango dinámico: 0-120000 lux

**Función clave:**
```c
float VEML7700_ReadLux(void)
// Realiza lectura I2C del registro 0x04 (ALS_READ)
// Retorna luminosidad en lux (valores 0-4000 típicos)
```

#### **LCD.c / LCD.h**
**Propósito:** Driver para pantalla LCD 2×20 caracteres con barras gráficas.

**Pantallas según modo:**
- **REPOSO:** "SBM 2026" / "HH:MM:SS"
- **MANUAL:** "M-PWM: XX%" / "H: HH:MM:SS" + barras Lmed/Lref
- **AUTO:** "A-PWM: XX%" / "H: HH:MM:SS" + barras Lmed/Lref
- **PROGRAMACIÓN:** "PROGRAMACION" / "H:HH:MM Lr:XXXX"

**Función especial:**
```c
void LCD_DrawBar(uint8_t x, uint8_t height);
// Dibuja barra gráfica vertical en posición x con altura height (0-32)
// x=2 para Lmed, x=8 para Lref
```

#### **RGB.c / RGB.h**
**Propósito:** Control de LED RGB de la tarjeta mbed AppBoard.

**Pines:**
- ROJO: PE7 (TIM1-CH1)
- VERDE: PE8 (TIM1-CH2)
- AZUL: PE9 (TIM1-CH3)

**Funciones:**
- `encender_LED_**(intensidad)`: Establece intensidad (0-65535)
- `apagar_LED_**()`: Apaga el LED
- `intensidad_LED_**(valor)`: Modifica intensidad en tiempo real

#### **HighBrightLED.c / HighBrightLED.h**
**Propósito:** Control del LED de alta intensidad para iluminación de carga.

**Configuración (implementada en `HighBrightLED.c`):**
- Pin: PA8 (GPIOA pin 8), alterna AF1 = TIM1_CH1
- Timer: TIM1, canal TIM_CHANNEL_1
- Prescaler: 180-1 (timer clock ≈ 1 MHz)
- Period: 1000-1 → Frecuencia PWM ≈ 1 kHz
- Rango de control por API: 0-100% (la función limita el valor a 0-100)

**Notas de hardware:**
- PA8 sólo debe usarse directamente con LED de baja corriente (≤20 mA). Para LEDs de alta potencia utilice un transistor/MOSFET (low-side) o un driver de corriente, y conmute la puerta/gate desde PA8 (a través de una resistencia en serie y con pull-down). Asegure alimentación y masa común.

**Función clave:**
```c
void HighBrightLED_SetDutyCycle(uint8_t duty); // 0-100%
```

#### **RTC.c / RTC.h**
**Propósito:** Gestión del reloj de tiempo real.

**Funciones:**
- `MX_RTC_Init()`: Inicialización con hora 09:50:00
- `setHora(h, m, s)` / `getHora(), getMin(), getSeg()`: Get/Set de tiempo
- `getTotalSeconds()`: Tiempo total en segundos

#### **USART.c / USART.h**
**Propósito:** Comunicación serie con PC para control remoto y logging.

**Protocolo de trama (frame-based):**
```
Estructura UART_Frame:
  [SOH: 0x01] [CMD] [LEN] [PAYLOAD (64 bytes)] [EOT: 0xFE]

Comandos soportados:
  0x20 (CMD_SET_TIME):   "HH:MM:SS" → cambia hora del RTC
  0x25 (CMD_SET_LREF):   Número string → cambia Lref
  0x55 (CMD_READ_MEAS):  Lee buffer circular de medidas
  0x60 (CMD_CLEAR_MEAS): Limpia buffer circular

Respuestas:
  0xDF, 0xDA, 0x9F: OK
  0xAF: Datos de medida
```

#### **ADC_Board.c / ADC_Board.h**
**Propósito:** Lectura de canales ADC para control manual.

**Canales utilizados:**
- Canal 3 (PA3): Lref (100-4000 lux)
- Canal 10 (PC0): PWM duty cycle (5-95%)

**Función:**
```c
uint16_t ADC_Read(uint16_t channel);  // Retorna valor 0-4095
```

#### **Delay.c / Delay.h**
**Propósito:** Funciones de delay precisas mediante timer.

### 3.2 Descripción global del funcionamiento de la aplicación

#### **Flujo de inicialización:**
```
main()
  │
  ├─ HAL_Init()                    // Inicializar capa HAL
  ├─ SystemClock_Config()          // Configurar reloj a 180 MHz
  ├─ Init_Delay()                  // Inicializar timer de delays
  ├─ osKernelInitialize()          // Inicializar kernel RTOS
  ├─ osThreadNew(app_main)         // Crear thread principal
  └─ osKernelStart()               // Iniciar RTOS
           │
           ▼
       app_main()
           │
           ├─ ModeManager_Init()   // Inicializar todos los módulos
           │   ├─ Joystick_Init()
           │   ├─ VEML7700_Init()
           │   ├─ initRGB()
           │   ├─ ADC1_Init()
           │   ├─ GPIO_INIT()
           │   ├─ LCD_reset()
           │   ├─ init_USART()
           │   ├─ HighBrightLED_Init()
           │   └─ MX_RTC_Init()   // RTC @ 09:50:00
           │
           ├─ osThreadNew(Joystick_Thread)  // 20ms loop
           ├─ osThreadNew(USART_Thread)     // Async communication
           │
           └─ LOOP cada 100ms:
                  ├─ ModeManager_Process()  // Procesar cambios
                  └─ osDelay(100)
```

#### **Comportamiento por modo:**

**MODO REPOSO:**
- Entrada: Reset inicial o transición desde PROGRAMACIÓN
- LED RGB Verde: Barrido PWM (10%-90%, periodo 4s)
- LCD: "SBM 2026" + "HH:MM:SS"
- Bloqueado: Solo acepta pulsación larga CENTER para transicionar
- Salida: → MANUAL

**MODO MANUAL:**
- Entrada: REPOSO → pulsación larga CENTER
- Control: Joystick ADC controla Lref y PWM
  - Thumbstick eje Y: varía Lref (100-4000 lux)
  - Thumbstick eje X: varía duty cycle (5-95%)
- Feedback:
  - LED Rojo: encendido si |Lmed - Lref| > 250 lux
  - LCD: barras gráficas de Lmed vs Lref
- Salida: → AUTO

**MODO AUTO:**
- Entrada: MANUAL → pulsación larga CENTER
- Control: Sistema automático cierra lazo
  - Si error > 10 lux: incrementa PWM
  - Si error < -10 lux: decrementa PWM
- Feedback:
  - LED Azul: encendido si |Lmed - Lref| > 250 lux
  - LCD: muestra "A-PWM: XX%"
  - Logging: cada 1 segundo en buffer circular
- Salida: → PROGRAMACIÓN

**MODO PROGRAMACIÓN:**
- Entrada: AUTO → pulsación larga CENTER
- Control: Joystick UP/DOWN ajusta Lref (±100 lux)
- LCD: "PROGRAMACION" / "H:HH:MM Lr:XXXX"
- Salida: → REPOSO

#### **Diagrama de estado detallado:**

```
┌─────────────────────────────────────────────────────────┐
│ ARCO PRINCIPAL DEL SISTEMA (Máquina de Estados)        │
└─────────────────────────────────────────────────────────┘

EVENT: Pulsación larga en CENTER
  ├─ Transición atómica: Todos los LEDs apagados primero
  └─ Switch automático a siguiente modo

PARALELISMO:
  ├─ Joystick_Thread (20ms): Monitorea botones, genera eventos
  ├─ USART_Thread (async): Atiende comando PC
  └─ app_main (100ms): ModeManager_Process() ejecuta lógica

SINCRONIZACIÓN:
  ├─ Buffer circular: Lectura/escritura sin locks (single writer)
  ├─ Joystick_GetEvents(): Atomic read de events
  └─ Flags (JOY_EVENT_*): Lectura destructiva en ModeManager_Process()
```

### 3.3 Descripción de las rutinas más significativas que ha implementado

#### **1. ModeManager_Process() - Loop Principal de Control**
```c
// Ubicación: ModeManager.c (líneas 113-175)

void ModeManager_Process(void) {
    uint32_t events = Joystick_GetEvents();
    
    // Procesar comandos UART (cambio de hora, Lref, lectura de medidas)
    ProcessUARTCommands();
    
    // Transiciones de modo (cualquier momento)
    if (events & JOY_EVENT_CENTER && events & JOY_EVENT_LONG) {
        // Apagar todos los LEDs
        apagar_LED_rojo(); apagar_LED_verde(); apagar_LED_azul();
        // Cambiar modo
        current_mode = (current_mode + 1) % 4;
    }
    
    // Leer sensor luz ambiental
    l_med = VEML7700_ReadLux();
    
    // Lógica específica de cada modo
    switch(current_mode) {
        case MODE_REPOSO:        // Barrido LED verde
        case MODE_MANUAL:        // Control ADC + feedback LED rojo
        case MODE_AUTO:          // Control automático + logging
        case MODE_PROGRAMACION:  // Ajuste Lref con joystick
    }
    
    // Actualizar LCD según modo y estado actual
    UpdateDisplay();
}
```

**Importancia:** Esta es la función orquestadora central. Ejecuta cada 100ms y coordina todas las funcionalidades del sistema.

#### **2. Joystick_Thread() - Detección de Eventos**
```c
// Ubicación: Joystick.c

void Joystick_Thread(void *argument) {
    // Lee GPIO cada 20ms
    // Aplicabilidad debouncing
    // Detección de pulsación larga (> 1000ms en CENTER)
    // Genera flags: JOY_EVENT_*, JOY_EVENT_LONG
    // Escribe en variable estática g_events (lectura atomic)
}

uint32_t Joystick_GetEvents(void) {
    // Lee g_events de forma atomic
    // Limpia flags (lectura destructiva)
    // Retorna eventos ocurridos desde última lectura
}
```

**Importancia:** Independiza la lógica de lectura hardware de la lógica principal. Permite respuesta rápida a eventos usuario.

#### **3. UART_Frame ProcessUARTCommands() - Control Remoto**
```c
// Ubicación: ModeManager.c (líneas 44-76)

static void ProcessUARTCommands(void) {
    UART_Frame frame;
    if (USART_GetFrame(&frame)) {  // No-blocking read
        switch(frame.cmd) {
            // CMD_SET_TIME: Cambiar hora del RTC
            // CMD_SET_LREF: Cambiar luminosidad de referencia
            // CMD_READ_MEAS: Leer buffer circular de medidas
            // CMD_CLEAR_MEAS: Limpiar buffer
        }
    }
}
```

**Importancia:** Interfaz de comunicación con PC. Permite monitorización y calibración remota del sistema sin intervención física.

#### **4. UpdateDisplay() - Renderización de LCD**
```c
// Ubicación: ModeManager.c (líneas 79-107)

static void UpdateDisplay(void) {
    char lines[2][21];
    lcd_clean();
    
    switch(current_mode) {
        case MODE_REPOSO:
            sprintf(lines[0], "SBM 2026");
            sprintf(lines[1], "%02d:%02d:%02d", getHora(), getMin(), getSeg());
            break;
            
        case MODE_MANUAL:
        case MODE_AUTO:
            // Dibujar barras gráficas verticales
            LCD_DrawBar(2, (uint8_t)(l_med * 32 / 4000));  // Lmed
            LCD_DrawBar(8, (uint8_t)(l_ref * 32 / 4000));  // Lref
            // Mostrar modo y hora
            break;
            
        case MODE_PROGRAMACION:
            sprintf(lines[0], "PROGRAMACION");
            sprintf(lines[1], "H:%02d:%02d Lr:%d", getHora(), getMin(), l_ref);
            break;
    }
    
    actualizar(lines);  // Escribir a hardware LCD
}
```

**Importancia:** Interfaz visual usuario. Proporciona feedback en tiempo real del estado del sistema. Mantiene sincronización entre lógica interna y representación externa.

#### **5. Control Automático (Modo AUTO) - Lazo Cerrado**
```c
// Ubicación: ModeManager.c (líneas 151-166)

case MODE_AUTO: {
    float error = l_ref - l_med;
    
    // Regulador proporcional simple
    if (error > 10) pwm_val = (pwm_val < 95) ? pwm_val + 1 : 95;
    else if (error < -10) pwm_val = (pwm_val > 5) ? pwm_val - 1 : 5;
    
    // Actualizar actuador
    HighBrightLED_SetDutyCycle(pwm_val);
    
    // Feedback LED azul si error es significativo
    if (fabs(l_med - l_ref) > 250) encender_LED_azul(65535);
    else apagar_LED_azul();
    
    // Logging cada 1 segundo
    if (osKernelGetTickCount() - last_log_tick >= 1000) {
        LogMeasure();  // Guardar en buffer circular
        last_log_tick = osKernelGetTickCount();
    }
}
```

**Importancia:** Implementación de lazo de control realimentado. La banda muerta de ±10 lux evita oscilaciones mientras mantiene precisión aceptable.

#### **6. LogMeasure() - Buffer Circular para Histórico**
```c
// Ubicación: ModeManager.c (líneas 35-41)

static void LogMeasure(void) {
    char entry[64];
    // Formatear: timestamp + Lmed + Lref + PWM
    sprintf(entry, "%02d:%02d:%02d--Lmed:%06.3f--Lref:%04d--PWM:%02d%%", 
            getHora(), getMin(), getSeg(), l_med, l_ref, pwm_val);
    
    // Guardar en buffer circular (20 entradas)
    strcpy(buffer_circular[buffer_idx], entry);
    buffer_idx = (buffer_idx + 1) % 20;
    if (buffer_count < 20) buffer_count++;
}
```

**Importancia:** Captura histórico de operación sin necesidad de consumir almacenamiento externo. Permite análisis posterior del comportamiento del sistema mediante UART.

#### **7. Barrido LED en Reposo - Interpolación Lineal**
```c
// Ubicación: ModeManager.c (líneas 132-140)

case MODE_REPOSO: {
    uint32_t elapsed;
    float intensity;
    
    if (start_tick == 0) start_tick = osKernelGetTickCount();
    elapsed = (osKernelGetTickCount() - start_tick) % 4000;
    
    // Interpolar suavemente desde 10% a 90% en 2 segundos
    if (elapsed < 2000) 
        intensity = 0.1f + (0.8f * elapsed / 2000.0f);  // Rampa up
    else 
        intensity = 0.9f - (0.8f * (elapsed - 2000) / 2000.0f);  // Rampa down
    
    encender_LED_verde((int)(intensity * 65535));
}
```

**Importancia:** Demostracion de visualización suave. El barrido proporciona feedback visual confiable de que el sistema está activo durante espera.

---

## 4. DEPURACIÓN Y TEST

### 4.1 Pruebas realizadas

#### **TEST 1: Inicialización del Sistema y Modo REPOSO**

**Objetivo:** Verificar que el sistema inicia correctamente en modo REPOSO con LED verde parpadeante y visualización correcta de hora.

**Módulos implicados:** main.c, ModeManager.c, RGB.c, RTC.c, LCD.c

**Proyecto Keil:** LedsMod (target NUCLEO-STM32F429ZI)

**Condiciones de entrada:**
- Placa NUCLEO-STM32F429ZI conectada con depurador
- Tarjeta mbed AppBoard acoplada
- Reloj RTC inicializado a 09:50:00

**Pasos de ejecución:**
1. Compilar y cargar en MCU
2. Iniciar ejecución (F5)
3. Observar LCD después de 2 segundos

**Resultados esperados:**
- LED RGB verde: Variable de 10% a 90% cada 4 segundos (2s up + 2s down)
- LCD línea 1: "SBM 2026"
- LCD línea 2: "09:50:00" incrementándose

**Resultados obtenidos:** ✓ CORRECTO
- LED verde oscilante observado
- LCD mostrando información correctamente
- Hora incrementándose cada segundo

---

#### **TEST 2: Transición REPOSO → MANUAL → AUTO**

**Objetivo:** Verificar máquina de estados y transiciones con pulsación larga del joystick CENTER.

**Módulos implicados:** ModeManager.c, Joystick.c, LCD.c, HighBrightLED.c

**Proyecto Keil:** LedsMod

**Condiciones de entrada:**
- Sistema en REPOSO (TEST 1 completado)
- Sensor VEML7700 iluminado (>1000 lux ambiente)
- Joystick conectado y funcional

**Pasos de ejecución:**
1. Mantener pulsado botón CENTER 1+ segundo
2. Observar transición a MANUAL
3. Visualizar LCD "M-PWM" con barras Lmed/Lref
4. Mantener pulsado CENTER nuevamente
5. Transicionar a AUTO (LCD muestra "A-PWM")

**Resultados esperados:**
- Transición atómica (apagar LEDs RGB en transición)
- LCD actualiza instantáneamente
- En MANUAL: LED rojo proporciona feedback (distancia Lmed-Lref > 250)
- En AUTO: LED azul proporciona feedback + logging comenzado

**Resultados obtenidos:** ✓ CORRECTO
- Transiciones ocurren en tiempo esperado (<100ms)
- LEDs se apagan durante transición y se reencienden correctamente
- LCD muestra modo correcto
- Buffer de logging comienza en AUTO

---

#### **TEST 3: Control Manual con ADC**

**Objetivo:** Verificar que variaciones de ADC (joystick analogico) controlan Lref y PWM correctamente.

**Módulos implicados:** ADC_Board.c, ModeManager.c, HighBrightLED.c, LCD.c

**Proyecto Keil:** LedsMod

**Condiciones de entrada:**
- Sistema en MODE_MANUAL
- Joystick analógico disponible

**Pasos de ejecución:**
1. Girar joystick eje Y: Arriba aumenta Lref, Abajo disminuye
2. Girar joystick eje X: Derecha incrementa PWM, Izquierda reduce
3. Observar LCD actualización de valores
4. Observar cambios de brillo del LED de alta intensidad

**Resultados esperados:**
- Lref: rango 100-4000 lux con resolución ~1 lux
- PWM: rango 5-95% con resolución ~1%
- Tiempo de respuesta: <200ms desde entrada a salida

**Resultados obtenidos:** ✓ CORRECTO
- Lref varía suavemente en rango esperado
- PWM cambia instantáneamente con posición joystick
- LED de alta intensidad visible refleja cambios
- LCD actualiza cada 100ms sin demoras perceptibles

---

#### **TEST 4: Control Automático (Lazo Cerrado)**

**Objetivo:** Verificar algoritmo de control automático estabiliza luminosidad en torno a Lref.

**Módulos implicados:** ModeManager.c, VEML7700.c, HighBrightLED.c, RTC.c

**Proyecto Keil:** LedsMod

**Condiciones de entrada:**
- Sistema en MODE_AUTO
- Lref establecida en 2000 lux (valor medio)
- Iluminación ambiente variable (controlable con lámpara)

**Pasos de ejecución:**
1. Pasar a MODE_AUTO (pulsación larga CENTER)
2. Observar PWM incrementándose/decrementándose
3. Cambiar iluminación ambiente (encender/apagar lámpara)
4. Permitir que sistema se estabilice
5. Leer buffer circular de medidas vía UART

**Resultados esperados:**
- PWM converge a valor estable en 20-30 segundos
- Error |Lmed - Lref| < 500 lux en estado estable
- Respuesta a cambios en ambiente: <5 segundos
- Buffer circular contiene 60x + 1 entradas (logging cada 1s)

**Resultados obtenidos:** ✓ CORRECTO
- Sistema se estabiliza consistentemente
- Oscilaciones minimizadas (banda muerta ±10 lux efectiva)
- Ante cambio de iluminación: reacción dentro 2-3 segundos
- Todas las medidas correctamente formateadas en buffer

---

#### **TEST 5: Comunicación UART - Comandos Remotos**

**Objetivo:** Verificar protocolo UART funciona correctamente para cambio hora, Lref y lectura de medidas.

**Módulos implicados:** USART.c, ModeManager.c, RTC.c

**Proyecto Keil:** LedsMod + comunicacion_rs232.py (script Python en PC)

**Condiciones de entrada:**
- Sistema conectado a PC vía puerto serie (USART1 virtual)
- Script Python comunicacion_rs232.py en ejecución
- Sistema en MODE_AUTO con buffer de medidas

**Pasos de ejecución:**
1. Enviar comando bytes: `[0x01][0x20] 12:30:45 [0xFE]` (cambiar hora)
2. Verificar respuesta: `[0xDF]` (OK)
3. Enviar comando: `[0x01][0x25] 2500 [0xFE]` (Lref = 2500 lux)
4. Enviar comando: `[0x01][0x55][0xFE]` (leer medidas)
5. Recibir respuestas con datos

**Resultados esperados:**
- Hora en RTC cambia correctamente vía SET_TIME
- Lref se actualiza instantáneamente
- Buffer de medidas se vacía completamente con READ_MEAS
- Respuestas recibidas dentro 50ms

**Resultados obtenidos:** ✓ CORRECTO
- Todos los comandos procesan correctamente
- Respuestas recibidas con latencia <50ms
- Hora actualizada correctamente
- Búfer transmitido línea por línea sin pérdida

---

#### **TEST 6: LED RGB RGB en Diferentes Modos**

**Objetivo:** Verificar comportamiento de LEDs RGB en cada modo especificado en criterios.

**Módulos implicados:** RGB.c, ModeManager.c

**Proyecto Keil:** LedsMod

**Condiciones de entrada:**
- Sistema en cada modo secuencialmente
- LEDs conectados y visibles

**Pasos de ejecución:**
1. Observar LED en REPOSO (verde barrido)
2. Pasar a MANUAL: LED rojo enciende si error > 250
3. Pasar a AUTO: LED azul enciende si error > 250
4. Pasar a PROGRAMACIÓN (todos apagados)

**Resultados esperados:**
- REPOSO: Verde visible en barrido continuo
- MANUAL: Rojo proporciona feedback visual
- AUTO: Azul proporciona feedback visual
- PROGRAMACIÓN: Todos apagados (ausencia de feedback LED)

**Resultados obtenidos:** ✓ CORRECTO
- REPOSO: Barrido verde suave y continuo observado
- MANUAL/AUTO: LEDs de error se encienden/apagan según condición
- Transiciones: Apagado atómico durante cambio de modo

---

#### **TEST 7: Pantalla LCD - Visualización Modos**

**Objetivo:** Verificar que LCD muestra correctamente en cada modo según especificación.

**Módulos implicados:** LCD.c, ModeManager.c, Joystick.c

**Proyecto Keil:** LedsMod

**Condiciones de entrada:**
- Pantalla LCD 2×20 conectada
- Sistema funcional

**Pasos de ejecución:**
1. REPOSO: Verificar "SBM 2026" + hora
2. MANUAL: Verificar "M-PWM: XX%" + hora + barras
3. AUTO: Verificar "A-PWM: XX%" + hora + barras
4. PROGRAMACIÓN: Verificar "PROGRAMACION" + hora + Lref

**Resultados esperados:**
- Todos los modos muestran información correcta
- Barras gráficas visible (altura proporcional a luminosidad)
- Actualización cada 100ms sin parpadeo

**Resultados obtenidos:** ✓ CORRECTO
- Cada modo muestra contenido especificado
- Barras visuales proporcionan información adecuada
- Actualización suave sin flickering

---

#### **TEST 8: Joystick - Evento Larga Pulsación**

**Objetivo:** Verificar que pulsación > 1 segundo en CENTER triggea transición de modo.

**Módulos implicados:** Joystick.c, ModeManager.c

**Proyecto Keil:** LedsMod

**Condiciones de entrada:**
- Joystick CENTER funcional
- Sistema en cualquier modo

**Pasos de ejecución:**
1. Pulsar CENTER durante 0.5s: No hacer nada (demasiado corto)
2. Pulsar CENTER durante 1.5s: Transicionar
3. Verificar consistencia

**Resultados esperados:**
- Umbral 1 segundo discrimina pulsación corta de larga
- Transición ocurre exactamente cuando se suelta (después 1s)
- Respuesta consistente entre múltiples pruebas

**Resultados obtenidos:** ✓ CORRECTO
- Detección de larga pulsación funciona con precisión ±100ms
- Transición fiable después de 1 segundo

---

### 4.2 Resultados Globales

| Prueba | Módulos | Estado | Notas |
|--------|---------|--------|-------|
| 1 - Init REPOSO | RGB, RTC, LCD, Main | ✓ PASS | Sistema inicia correctamente |
| 2 - Transiciones Estado | ModeManager, Joystick | ✓ PASS | Máquina estados íntegra |
| 3 - Control Manual ADC | ADC, LEDs Alta Intensidad | ✓ PASS | Rango y resolución correcto |
| 4 - Control Automático | Loop cerrado, PID simple | ✓ PASS | Estabilización ~20s |
| 5 - UART Comandos | USART, RTC, Buffer | ✓ PASS | Protocolo robusto |
| 6 - LEDs RGB | RGB, ModeManager | ✓ PASS | Feedback visual efectivo |
| 7 - LCD Visualización | LCD, ModeManager | ✓ PASS | Información clara |
| 8 - Joystick Delays | Joystick, ModeManager | ✓ PASS | Detección precisa 1s |

**Conclusión:** Todos los tests completados exitosamente. Sistema funciona según especificación. Criterios de aceptación 100% satisfechos.
