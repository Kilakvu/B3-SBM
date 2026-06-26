# Guía de Estudio — Revisión de Código B3-SBM

## Sistema de Control de Iluminación Ambiental (STM32F429ZI)

---

## 1. Configuración de Reloj (Clock)

### Preguntas típicas

**P: ¿Qué frecuencia de reloj usa el sistema y cómo se obtiene?**
R: 180 MHz. Se parte del **HSI** (High Speed Internal — oscilador interno de 16 MHz), se usa el **PLL** (Phase-Locked Loop — bucle que multiplica la frecuencia de entrada) con unos divisores llamados PLLM=8, PLLN=180, PLLP=2.
Primero el **VCO** (Voltage-Controlled Oscillator — oscilador interno del PLL) genera:
`f_VCO = 16 MHz × 180 / 8 = 360 MHz`
Luego se divide entre 2 para obtener la frecuencia del sistema:
`f_SYS = 360 / 2 = 180 MHz`

**P: ¿Por qué 180 MHz y no otra frecuencia?**
R: Es la frecuencia máxima a la que puede funcionar el microcontrolador STM32F429ZI según su hoja de datos (referencia RM0090), cuando trabaja a 1.25 V. Se aprovecha toda la capacidad del procesador Cortex-M4 sin overclock.

**P: ¿Qué son los buses APB1 y APB2 y a qué frecuencia van?**
R: Son buses internos que conectan el procesador con los periféricos:
- **APB1** (bus de periféricos lentos): 45 MHz (se obtiene dividiendo 180 MHz entre 4). Aquí van periféricos como UART, I2C, RTC.
- **APB2** (bus de periféricos rápidos): 90 MHz (180 MHz ÷ 2). Aquí van los timers y el ADC.

**P: ¿Por qué el timer TIM1 puede funcionar a 180 MHz si está en APB2 que va a 90 MHz?**
R: Cuando el divisor del bus APB2 es mayor que 1 (en este caso ÷2), el hardware duplica automáticamente la frecuencia que llega a los timers: 90 MHz × 2 = 180 MHz. Es una característica del diseño del microcontrolador para que los timers puedan funcionar a mayor velocidad que los periféricos normales.

**P: ¿Qué pasaría si usáramos HSE (High Speed External — cristal de cuarzo externo) en vez de HSI?**
R: El HSE es más preciso y estable frente a cambios de temperatura. El HSI tiene una precisión de ±1%, que es suficiente para UART y RTC, pero no para aplicaciones que requieran sincronización precisa como CAN o Ethernet.

---

## 2. PWM (Pulse Width Modulation — Modulación por Ancho de Pulso)

### Preguntas típicas

**P: ¿Qué frecuencia tiene la señal PWM del LED de alta intensidad?**
R: 1 kHz. Se configura usando el **TIM1** (Timer 1) con un **prescaler** (predivisor) de 180-1, que reduce la frecuencia base de 180 MHz a aproximadamente 1 MHz, y un **auto-reload** (valor máximo de cuenta) de 1000-1, de forma que 1 MHz / 1000 = 1 kHz.

**P: ¿Por qué 1 kHz y no 100 Hz o 10 kHz?**
R: 
- Por debajo de 100 Hz el ojo humano ve el parpadeo.
- Entre 100 Hz y 1 kHz hay frecuencias audibles que podrían producir zumbidos en componentes electrónicos (bobinas, condensadores).
- 1 kHz está dentro del rango donde el LED responde bien, y da 1000 pasos de resolución (~0.1% por paso), que es suficiente para un control suave de brillo.

**P: ¿Qué resolución tiene el PWM?**
R: 1000 pasos distintos, aproximadamente 0.1% por paso. Se controla con la función `HighBrightLED_SetDutyCycle(valor)`, que acepta valores enteros entre 0 y 100 (porcentaje).

**P: ¿Qué pin y timer se usan para el LED de alta intensidad?**
R: Se usa **PE9** con **TIM1_CH1** (canal 1 del timer 1 como función alternativa). Originalmente se pensó en PA8, pero se cambió a PE9 porque ese pin está marcado como D6 en los conectores de la placa NUCLEO y es más accesible para conectar el LED externo.

**P: ¿Qué diferencia hay entre un timer avanzado como TIM1 y uno básico como TIM6?**
R: TIM1 es un **timer avanzado**: tiene canales PWM independientes, puede generar señales complementarias (para control de motores), tiene "dead-time" (tiempo muerto entre conmutaciones para evitar cortocircuitos), y protección por hardware. TIM6 es un **timer básico** que solo sirve para contar tiempo o generar delays.

---

## 3. ADC (Analog-to-Digital Converter — Conversor Analógico-Digital)

### Preguntas típicas

**P: ¿Qué resolución tiene el ADC?**
R: 12 bits, lo que significa que divide la tensión de entrada (0 a 3.3 V) en 4096 niveles posibles (valores de 0 a 4095).

**P: ¿Por qué se usa el canal 3 (pin PA3) para Lref (luminosidad de referencia) y el canal 10 (pin PC0) para PWM?**
R: Porque esos pines están conectados al joystick analógico de la **mbed AppBoard** (placa de expansión). PA3 corresponde al **eje Y** del joystick (movimiento arriba/abajo = controla Lref), y PC0 corresponde al **eje X** (izquierda/derecha = controla el duty cycle del PWM).

**P: ¿Cómo se convierten los valores del ADC (0-4095) a lux o porcentaje?**
R: Con una regla de tres lineal (mapeo lineal):
```
Lref = 100 + (ADC_raw × 3900 / 4095)   → da valores entre 100 y 4000 lux
PWM  = 5   + (ADC_raw × 90   / 4095)   → da valores entre 5% y 95%
```

**P: ¿Por qué Lref empieza en 100 lux y no en 0?**
R: Porque 0 lux es oscuridad total, que no tiene sentido como referencia de iluminación ambiental. Arrancar en 100 lux asegura que el sistema tenga siempre un valor de referencia válido desde el inicio.

---

## 4. I2C (Inter-Integrated Circuit) y Sensor VEML7700

### Preguntas típicas

**P: ¿A qué velocidad funciona el bus I2C?**
R: 100 kHz (modo estándar). Es suficiente porque el sensor VEML7700 solo necesita unos 200 microsegundos por lectura.

**P: ¿Por qué no usar 400 kHz (Fast Mode)?**
R: El sensor soporta 400 kHz, pero 100 kHz da más margen frente a ruido y problemas con cables largos o mala conexión en la placa de pruebas. Es más fiable.

**P: ¿Qué registros internos del VEML7700 se usan y para qué sirven?**
R:
- **0x00 (ALS_CONF)**: Registro de configuración — se programa la resolución (0.0036 lux por paso) y el modo de medición continua.
- **0x04 (ALS_READ)**: Registro de lectura de luz ambiental — se lee para obtener el valor de luminosidad.
- **0x05 (WHITE_READ)**: Registro de componente blanco — no se usa en este proyecto.

**P: ¿Cuál es la resolución del sensor VEML7700?**
R: 0.0036 lux por paso, con un rango de 0 a 120000 lux. En interiores se miden típicamente entre 100 y 4000 lux.

**P: ¿Por qué se lee el sensor cada 100 ms y no de forma continua?**
R: El sensor tiene un tiempo de integración interno (típicamente 25-50 ms) durante el cual acumula luz para hacer la medición. Leer más rápido no daría valores nuevos; leer más lento retrasaría el lazo de control automático.

---

## 5. RTC (Real-Time Clock — Reloj de Tiempo Real)

### Preguntas típicas

**P: ¿Qué fuente de reloj usa el RTC y por qué?**
R: Usa **LSE** (Low Speed External — oscilador externo lento) con un cristal de cuarzo de **32.768 kHz**. Se elige este cristal porque los relojes de tiempo real necesitan una fuente que funcione incluso en modo de bajo consumo.

**P: ¿Por qué 32.768 kHz exactamente?**
R: Porque 32.768 es una potencia de 2 (2¹⁵). Esto permite dividirlo exactamente por 32768 mediante un contador de 15 bits para obtener exactamente 1 pulso por segundo (1 Hz).

**P: ¿El RTC mantiene la hora aunque se reinicie el microcontrolador?**
R: En esta placa no, porque no está conectada la pila de respaldo (pin VBAT). Si se hace un reset o se apaga la alimentación, el RTC se pierde. No hay requisito de persistencia en este proyecto.

**P: ¿Qué formato de hora se usa?**
R: Formato de 24 horas. El reloj se inicializa a las 09:50:00.

**P: ¿Cómo se cambia la hora desde el PC vía UART?**
R: Mediante el comando `CMD_SET_TIME (0x20)`, enviando en el payload la hora en formato texto "HH:MM:SS". El microcontrolador parsea esa cadena con la función `sscanf` y llama a `setHora()`.

---

## 6. UART (Universal Asynchronous Receiver-Transmitter) y Protocolo Serie

### Preguntas típicas

**P: ¿Qué estructura tiene la trama (paquete) que se envía por UART?**
R: `[SOH: 0x01] [CMD] [LEN] [PAYLOAD (hasta 64 bytes)] [EOT: 0xFE]`
- **SOH** (Start Of Header): Marca el inicio de una trama (byte 0x01).
- **CMD**: Byte que identifica el comando.
- **LEN**: Longitud del payload.
- **PAYLOAD**: Datos del comando (hasta 64 bytes).
- **EOT** (End Of Transmission): Marca el final (byte 0xFE).

**P: ¿Por qué se usa un protocolo estructurado en vez de enviar texto plano (como AT+COMANDO)?**
R: Un protocolo con delimitadores permite:
- Saber dónde empieza y termina cada mensaje (SOH/EOT), incluso si hay ruido o retrasos.
- Detectar tramas incompletas o corruptas.
- Usar un solo byte para elegir entre 256 comandos posibles.
- Separar los problemas de comunicación (ruido en el cable) de los problemas de datos (payload incorrecto).

**P: ¿Qué comandos soporta el sistema?**
- **0x20** (Set time): Cambia la hora del RTC.
- **0x25** (Set Lref): Cambia la luminosidad de referencia.
- **0x55** (Read measurements): Lee el buffer circular con el histórico de medidas.
- **0x60** (Clear measurements): Limpia el buffer circular.

**P: ¿Cómo se maneja que el programa principal y la recepción UART compartan datos sin pisarse?**
R: La **USART_Thread** se ejecuta de fondo y va recibiendo tramas. Cuando `ModeManager_Process()` se ejecuta (cada 100 ms), llama a `ProcessUARTCommands()` que mira si hay tramas pendientes y las procesa sin bloquearse (modo **no-blocking** — si no hay trama, sigue adelante).

**P: ¿Por qué se usan bytes específicos como 0xDF, 0xDA, 0x9F para respuestas de éxito?**
R: Para que el programa en el PC pueda distinguir claramente si la respuesta es un "OK" o son datos de medida (que usan el prefijo 0xAF). Se evita cualquier ambigüedad en la comunicación.

---

## 7. RTOS (Real-Time Operating System — Sistema Operativo de Tiempo Real)

### Preguntas típicas

**P: ¿Qué tareas (hilos/threads) existen en el sistema?**
R: Hay 3 threads:
1. **app_main** (bucle principal, se ejecuta cada 100 ms): Orquesta todo el sistema, lee sensores, ejecuta control, actualiza pantalla.
2. **Joystick_Thread** (cada 20 ms): Lee los botones del joystick, aplica **debouncing** (eliminación de rebotes mecánicos) y detecta pulsaciones largas.
3. **USART_Thread** (asíncrono — se ejecuta cuando llegan datos): Atiende la comunicación serie con el PC.

**P: ¿Por qué el joystick necesita su propio hilo en vez de leerse desde el bucle principal?**
R: Para independizar la lectura del hardware (los botones hay que leerlos cada pocos milisegundos para no perderse pulsaciones) de la lógica principal (que se ejecuta cada 100 ms). Si el joystick se leyera desde app_main, los botones pulsados durante menos de 100 ms se perderían.

**P: ¿Cómo se sincronizan los hilos entre sí?**
R: Usando variables compartidas (`g_events`) que se leen de forma **atómica** (en una sola operación, sin posibilidad de interrupción). El joystick escribe los eventos y `Joystick_GetEvents()` los lee de una vez. Se usa además el patrón **single writer** (un solo escritor) para el buffer circular, evitando complicaciones.

**P: ¿Por qué no se usan semáforos o mutex (bloqueos de exclusión mutua)?**
R: Porque no hacen falta. La comunicación entre hilos es muy simple: el joystick escribe una variable de tipo `uint32_t` (32 bits), y el main la lee entera de una vez. En un microcontrolador de 32 bits, leer o escribir un `uint32_t` es una operación atómica (no se puede interrumpir a medias).

**P: ¿Qué pasa si el hilo del joystick se retrasa y no se ejecuta durante 40 ms?**
R: Podría perderse una pulsación muy corta (<20 ms). Pero el sistema está diseñado con margen: la pulsación larga que cambia de modo requiere mantener el botón 1 segundo completo, así que 40 ms de retraso no afectan.

---

## 8. Máquina de Estados (ModeManager)

### Preguntas típicas

**P: ¿Qué modos de funcionamiento existen y cómo se pasa de uno a otro?**
R: Hay 4 modos en ciclo cerrado: **REPOSO → MANUAL → AUTO → PROGRAMACIÓN → REPOSO**. Para cambiar de uno a otro siempre se usa la misma acción: pulsar el botón CENTER del joystick durante más de 1 segundo (pulsación larga).

**P: ¿Por qué es necesario un modo REPOSO?**
R: Actúa como **bloqueo de seguridad** — el sistema no puede empezar a funcionar activamente sin que el usuario pulse deliberadamente durante 1 segundo. También sirve para demostrar que el sistema está vivo: el LED verde hace un barrido de brillo (sube y baja suavemente).

**P: ¿Por qué la transición entre modos apaga todos los LEDs RGB antes de encender los del nuevo modo?**
R: Para evitar que se vea el LED del modo anterior encendido durante unos milisegundos mientras se cambia. Se hace una transición "limpia" (atómica): todo apagado, luego enciendo lo que corresponda.

**P: ¿Qué hace cada modo?**

| Modo | LED RGB | LCD | Control |
|---|---|---|---|
| **REPOSO** | Verde (barrido suave 10%-90% cada 4 s) | "SBM 2026" + hora actual | Bloqueado — no hace nada |
| **MANUAL** | Rojo (se enciende si error > 250 lux) | M-PWM + barras Lmed/Lref + hora | Joystick analógico controla Lref y PWM |
| **AUTO** | Azul (se enciende si error > 250 lux) | A-PWM + barras Lmed/Lref + hora | Lazo cerrado: ajusta PWM automáticamente |
| **PROGRAMACIÓN** | Todos apagados | "PROGRAMACION" + hora + Lref | Joystick UP/DOWN ajusta Lref (±100 lux) |

**P: ¿Qué ocurre si el joystick analógico falla mientras se está en modo MANUAL?**
R: El sistema simplemente mantiene el último valor de PWM y Lref que tenía. No pasa nada catastrófico porque el control manual es de **bucle abierto** (no hay realimentación que pueda descontrolarse).

**P: ¿Por qué en PROGRAMACIÓN todos los LEDs están apagados?**
R: Por diseño. En este modo no se necesita feedback visual de error porque el usuario está configurando manualmente el valor de Lref. No hay control activo que requiera indicación.

---

## 9. Control Automático (Lazo Cerrado)

### Preguntas típicas

**P: ¿Qué tipo de controlador se usa en el modo AUTO?**
R: Un **controlador proporcional con banda muerta** (solo reacciona cuando el error supera un umbral). Es el tipo más simple de control automático.

**P: ¿Cuál es la ecuación de control?**
```c
error = Lref - Lmed;  // Diferencia entre lo que queremos y lo que hay
if (error > 10) pwm_val = min(pwm_val + 1, 95);   // Sube PWM si falta luz
else if (error < -10) pwm_val = max(pwm_val - 1, 5); // Baja PWM si sobra luz
// Si error está entre -10 y +10, no se hace nada (zona muerta)
```

**P: ¿Por qué se usa una banda muerta de ±10 lux?**
R: Para evitar **jitter** (oscilación rápida e irritante del LED alrededor del valor deseado). Sin banda muerta, el sistema estaría continuamente subiendo y bajando 1% de PWM porque el sensor tiene pequeñas variaciones de lectura. Con ±10 lux de tolerancia, el sistema se queda quieto cuando está "suficientemente cerca".

**P: ¿Por qué no se usa un controlador PID completo (Proporcional-Integral-Derivativo)?**
R: El sistema no lo necesita. El LED responde en ~2-3 segundos y la precisión requerida es de ±500 lux, que es suficiente para iluminación ambiental. Un PID añadiría tres parámetros que habría que ajustar experimentalmente (sintonizar) sin una mejora notable en el resultado.

**P: ¿Cada cuánto se ejecuta el lazo de control?**
R: Cada 100 ms, en cada llamada a `ModeManager_Process()`.

**P: ¿Por qué 100 ms y no 10 ms o 1 segundo?**
R: 
- Si fuera cada 10 ms, leeríamos el sensor demasiado rápido y obtendríamos el mismo valor varias veces (el VEML7700 tarda 25-50 ms en dar una lectura nueva).
- Si fuera cada 1 segundo, el sistema tardaría mucho en reaccionar ante cambios de luz.
- 100 ms es un buen equilibrio: el sensor tiene tiempo de integrar, el LED cambia suavemente, y la respuesta es rápida.

**P: ¿Qué rango de PWM se permite usar?**
R: Entre **5%** y **95%**. No se permite llegar a 0% ni a 100% para evitar que el LED se apague del todo o se sature al máximo, manteniendo siempre un margen de control.

---

## 10. Buffer Circular (Logging — Registro de Medidas)

### Preguntas típicas

**P: ¿Cómo está implementado el buffer circular?**
R: Es un array de 20 filas por 64 columnas: `buffer_circular[20][64]`. Usa un índice (`buffer_idx`) que apunta a la siguiente posición libre, y un contador (`buffer_count`) que lleva la cuenta de cuántas entradas hay almacenadas.

**P: ¿Por qué solo 20 entradas?**
R: Por limitación de memoria RAM. 20 entradas × 64 bytes = 1280 bytes (aproximadamente 1.2 KB). Es suficiente para guardar 20 segundos de histórico (se guarda 1 entrada por segundo), sin consumir demasiada memoria.

**P: ¿Qué información se guarda en cada entrada?**
R: Una línea de texto con: `"HH:MM:SS--Lmed:XXXX.XXX--Lref:XXXX--PWM:XX%"` — es decir, la hora exacta, la luminosidad medida, la luminosidad de referencia, y el porcentaje de PWM aplicado.

**P: ¿Cómo se evita que los datos se corrompan si dos partes del programa intentan acceder al buffer a la vez?**
R: Solo `ModeManager_Process()` escribe en el buffer (patrón **single writer** — un único escritor). El comando `CMD_READ_MEAS` (lectura remota desde el PC) utiliza `memcpy` para copiar todo el contenido de una sola vez, evitando que una escritura a medias pueda dejar datos inconsistentes.

---

## 11. LCD (Liquid Crystal Display) y Visualización

### Preguntas típicas

**P: ¿Qué tamaño tiene la pantalla LCD?**
R: 2 líneas de texto, cada una de 20 caracteres (2×20).

**P: ¿Cómo se dibujan las barras gráficas verticales que representan Lmed y Lref?**
R: Mediante la función `LCD_DrawBar(x, altura)`, donde la altura se calcula como `l_med × 32 / 4000` para escalar el valor de 0-4000 lux a 0-32 píxeles. La barra de Lmed se dibuja en la columna 2 (x=2) y la de Lref en la columna 8 (x=8).

**P: ¿Por qué se usa `sprintf` (una función de formateo de cadenas pesada) en un sistema embebido con recursos limitados?**
R: Porque es la forma más legible y fácil de mantener para construir cadenas de texto. Ocupa unos 2-3 KB de memoria Flash, que es aceptable cuando el microcontrolador tiene 2 MB de Flash disponibles.

**P: ¿Cada cuánto se actualiza la pantalla LCD?**
R: Cada 100 ms, dentro de `UpdateDisplay()`, que es llamada por `ModeManager_Process()`.

---

## 12. Joystick y Debouncing (Eliminación de Rebotes)

### Preguntas típicas

**P: ¿Cada cuánto tiempo se lee el estado del joystick?**
R: Cada 20 ms, que es el periodo de ejecución del **Joystick_Thread**.

**P: ¿Qué es el debouncing y cómo se implementa?**
R: Los botones mecánicos, cuando se pulsan o sueltan, hacen contacto en varias ocasiones muy rápido (rebotes) antes de estabilizarse. El **debouncing** consiste en esperar un tiempo (20 ms) entre lecturas para asegurarse de que el botón se ha estabilizado. Si el estado del botón cambia, se confirma en la siguiente lectura antes de actuar.

**P: ¿Cómo se detecta la pulsación larga (más de 1 segundo)?**
R: El hilo del joystick mide el tiempo que el botón CENTER está presionado. Cuando ese tiempo supera los 1000 ms, activa una bandera especial llamada `JOY_EVENT_LONG`.

**P: ¿Por qué el umbral de pulsación larga es exactamente 1 segundo?**
R: Es un estándar de usabilidad para interfaces físicas. Menos de 500 ms se confunde fácilmente con una pulsación accidental; más de 2 segundos resulta incómodo para el usuario.

**P: ¿Qué pines del microcontrolador usa el joystick?**
R: Cada dirección del joystick tiene su propio pin de entrada digital:
- **PE12**: Abajo (DOWN)
- **PE14**: Izquierda (LEFT)
- **PE15**: Centro (CENTER)
- **PB10**: Arriba (UP)
- **PB11**: Derecha (RIGHT)

---

## 13. Hardware y Pines (Conexiones Físicas)

### Preguntas típicas

**P: ¿Qué placa o hardware se usa?**
R: La placa principal es una **NUCLEO-STM32F429ZI** (placa de desarrollo de STMicroelectronics), con una **mbed AppBoard** acoplada encima que añade el joystick, la pantalla LCD, el LED RGB y el sensor de luz.

**P: ¿Qué periféricos están conectados a qué pines del microcontrolador?**

| Periférico | Pin(es) | Periférico interno del micro |
|---|---|---|---|
| Comunicación serie con PC | PD8 (TX), PD9 (RX) | USART3 |
| Sensor de luz VEML7700 | PB8 (SCL — reloj), PB9 (SDA — datos) | I2C1 |
| LEDs RGB (rojo, verde, azul) | PD13 (R), PD12 (G), PD11 (B) | TIM4 — canales 2/1 para PWM en R/G; B es digital ON/OFF |
| LED de alta intensidad | PE9 | TIM1 — canal 1 |
| Joystick analógico (ejes X/Y) | PA3 (eje Y), PC0 (eje X) | ADC1 — canales 3 y 10 |
| Pantalla LCD | PA7 (MOSI), PA5 (SCK), PD14 (CS), PF13 (A0), PA6 (RESET) | SPI1 + GPIO |

**P: ¿Por qué se usa USART3 y no USART1 o USART2?**
R: Porque en la placa NUCLEO-STM32F429ZI, el **ST-Link** (programador/depurador USB) está conectado internamente a **USART3** en los pines PD8 (TX) y PD9 (RX). Esa es la forma de tener un puerto COM virtual al conectar la placa por USB.

**P: ¿Qué timer se usa para los LEDs RGB y por qué?**
R: Se usa **TIM4** para los canales PWM de los LEDs rojo (PD13, TIM4_CH2) y verde (PD12, TIM4_CH1). El azul (PD11) se controla como GPIO digital (ON/OFF) porque no necesita PWM.

**P: ¿El pin PE9 del LED de alta intensidad puede conectarse directamente a un LED potente?**
R: **No.** PE9 solo puede suministrar 20 mA como máximo, que es poco para un LED de alta potencia. Para LEDs potentes hay que usar un transistor MOSFET externo que conmute la corriente grande desde una fuente externa, controlado por la señal de PE9.

---

## 14. Preguntas de Arquitectura General

**P: ¿Cuál es la estructura general del sistema?**
R: Es una arquitectura basada en **RTOS** con tres hilos de ejecución:
1. Un hilo principal (app_main, ejecutándose cada 100 ms) que coordina todo.
2. Un hilo de entrada (joystick, cada 20 ms) que lee los botones.
3. Un hilo de comunicación (USART, asíncrono) que atiende al PC.
Todo ello coordinado por el módulo **ModeManager**, que implementa una máquina de estados de 4 modos.

**P: ¿Por qué se eligió CMSIS-RTOS2 (el sistema operativo de ARM) en vez de FreeRTOS?**
R: Porque CMSIS-RTOS2 es el estándar oficial de ARM para sistemas operativos de tiempo real en microcontroladores Cortex-M. El entorno Keil MDK lo integra de serie, proporciona una API uniforme y funciona perfectamente con el depurador.

**P: ¿Qué mejoras se podrían hacer al sistema?**
- Añadir una pila/batería externa para que el RTC no pierda la hora al apagar la placa.
- Implementar un controlador PID completo con ganancias ajustables desde UART.
- Usar **DMA** (Direct Memory Access — acceso directo a memoria) para la UART, así el microcontrolador no tiene que ocuparse de cada byte que llega.
- Añadir detección de errores en la comunicación I2C con reintentos automáticos.
- Guardar el histórico de medidas en una memoria EEPROM externa para que no se pierda al apagar.

**P: ¿Cómo se podría optimizar el uso de memoria?**
- Usar formato binario en lugar de texto para el logging (ocuparía menos espacio).
- Evitar `sprintf` y formatear las cadenas manualmente.
- Ajustar el tamaño de la pila (**stack**) de cada hilo al mínimo necesario.
- Usar variables locales en vez de globales cuando sea posible.

---

## 15. Preguntas de Casos Límite (Situaciones Extremas)

**P: ¿Qué pasa si el sensor VEML7700 no responde por el bus I2C (por ejemplo, está desconectado o estropeado)?**
R: La función `VEML7700_ReadLux()` devolvería 0 o valores incorrectos. El sistema intentaría compensar esta "oscuridad" subiendo el PWM al máximo (`error = Lref - 0 = Lref`), y el LED de alta intensidad se encendería al 95% permanentemente. La implementación actual **no** detecta este fallo.

**P: ¿Qué ocurre si el usuario mantiene pulsado el botón CENTER todo el rato sin soltarlo?**
R: El sistema cambia de modo una sola vez. Aunque el botón siga pulsado, no se genera otro evento de pulsación larga hasta que el usuario suelte el botón y vuelva a pulsarlo (el **debouncing** impide que se detecten eventos repetidos mientras está pulsado).

**P: ¿Qué pasa si se recibe un comando UART mal formado (le faltan bytes, tiene datos incorrectos, etc.)?**
R: El protocolo de trama verifica que el mensaje empiece por SOH (0x01), termine por EOT (0xFE) y tenga la longitud correcta. Si algo falla, la trama completa se ignora y se espera la siguiente. El sistema no se bloquea ni se reinicia.

**P: ¿El buffer circular de 20 entradas puede llenarse y desbordarse?**
R: No, porque está implementado como **buffer circular** (anillo). Cuando está lleno y se guarda una nueva entrada, se sobrescribe la más antigua. El contador `buffer_count` nunca supera el valor de 20.

---

## 16. Conceptos Clave para la Defensa

### Justificaciones de diseño que debes poder explicar con tus propias palabras:

1. **Banda muerta de ±10 lux**: Si no existiera, el sistema estaría continuamente subiendo y bajando el PWM porque el sensor da lecturas ligeramente diferentes cada vez. Con la banda muerta, cuando la luz medida está cerca de la deseada (diferencia menor de 10 lux), el sistema se queda quieto. Es un compromiso entre precisión y estabilidad.

2. **Periodo de 100 ms del bucle principal**: Tiene que ser más lento que el tiempo que tarda el sensor en medir (25-50 ms) pero más rápido de lo que tarda el LED en responder (2-3 segundos). 100 ms cumple ambas condiciones.

3. **Hilo separado para el joystick**: Si el joystick se leyera desde el bucle principal que va a 100 ms, las pulsaciones de menos de 100 ms se perderían. Al tener un hilo propio que lee cada 20 ms, podemos detectar cualquier pulsación, por corta que sea.

4. **Protocolo UART con tramas delimitadas**: Si se enviara texto plano sin estructura, sería imposible saber dónde acaba un mensaje y empieza el siguiente si hay ruido o retrasos en la comunicación. Con SOH/EOT y longitud, cada mensaje está perfectamente delimitado.

5. **Apagar todos los LEDs al cambiar de modo**: Garantiza que nunca se vea el LED del modo anterior encendido brevemente después de la transición. Es una cuestión de calidad de la experiencia de usuario.

6. **Modo REPOSO como pantalla de bloqueo**: Evita que el sistema empiece a funcionar (encender LEDs, controlar iluminación) sin que el usuario haya demostrado intención explícita manteniendo pulsado 1 segundo.

### Fórmulas clave (con explicación):

```
CLOCK (Reloj del sistema):
  f_VCO = f_HSI × PLLN / PLLM = 16 × 180 / 8 = 360 MHz
  (El oscilador interno de 16 MHz se multiplica hasta 360 MHz dentro del PLL)
  
  f_SYS = f_VCO / PLLP = 360 / 2 = 180 MHz
  (Luego se divide entre 2 para obtener la frecuencia final del sistema)

PWM (Modulación por ancho de pulso):
  f_PWM = f_TIM / (Prescaler × Period) = 1 MHz / 1000 = 1 kHz
  (El timer cuenta de 0 a 999 a 1 MHz, generando una señal que se repite 1000 veces por segundo)

ADC (Conversión analógico-digital):
  valor_físico = mínimo + (valor_ADC × (máximo - mínimo) / 4095)
  (Regla de tres para convertir el número 0-4095 del ADC a lux o porcentaje)

BARRIDO LED VERDE (interpolación lineal):
  Si elapsed < 2000 ms: intensidad = 0.1 + 0.8 × (elapsed / 2000)       [subiendo]
  Si elapsed ≥ 2000 ms: intensidad = 0.9 - 0.8 × ((elapsed-2000) / 2000) [bajando]
  (El LED va del 10% al 90% de brillo en 2 segundos, y vuelve en otros 2)

CONTROL AUTOMÁTICO:
  error = Lref - Lmed
  Si error > +10: subir PWM 1%
  Si error < -10: bajar PWM 1%
  (Si la diferencia entre lo deseado y lo medido es pequeña, no se hace nada)
```

### Valores numéricos clave que deberías recordar:

| Parámetro | Valor | Para qué sirve |
|---|---|---|
| **Frecuencia del sistema** | 180 MHz | Velocidad a la que funciona el procesador |
| **Frecuencia del PWM** | 1 kHz | Número de veces por segundo que se enciende/apaga el LED |
| **Periodo de lectura del joystick** | 20 ms | Cada cuánto se miran los botones |
| **Periodo del bucle principal** | 100 ms | Cada cuánto se lee el sensor y se actualiza todo |
| **Banda muerta del control** | ±10 lux | Tolerancia antes de ajustar el PWM |
| **Umbral del LED de error** | >250 lux | Cuándo se encienden los LEDs rojo/azul de feedback |
| **Rango de Lref** | 100-4000 lux | Valores posibles de luminosidad de referencia |
| **Rango del PWM** | 5-95% | Límites del duty cycle del LED de alta intensidad |
| **Resolución del ADC** | 12 bits (0-4095) | Número de niveles distintos que puede medir |
| **Velocidad del bus I2C** | 100 kHz | Velocidad de comunicación con el sensor de luz |
| **Dirección I2C del VEML7700** | 0x10 | Dirección del sensor en el bus |
| **Resolución del sensor** | 0.0036 lux/step | Precisión con la que mide la luz |
| **Cristal del RTC** | 32.768 kHz | Frecuencia del reloj externo que da la hora |
| **Tamaño del buffer circular** | 20 × 64 bytes | Capacidad del histórico de medidas |
| **Periodo del barrido verde** | 4 s (2 subiendo + 2 bajando) | Tiempo que tarda el LED en hacer un ciclo completo |
| **Rango del barrido verde** | 10%-90% | Brillo mínimo y máximo durante el barrido |
| **Umbral de pulsación larga** | 1 s | Tiempo que hay que mantener CENTER para cambiar de modo |
| **Tiempo de debouncing** | 20 ms | Espera para asegurar que el botón se ha estabilizado |
| **Hora inicial del RTC** | 09:50:00 | Hora con la que arranca el reloj |
