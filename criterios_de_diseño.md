# Historia de Usuario: Gestión de Estados y Modos de Operación

**Como** usuario del sistema de control de iluminación ambiental
**Quiero** poder cambiar secuencialmente entre los distintos modos de trabajo del sistema (REPOSO, MANUAL, AUTO y PROGRAMACIÓN) utilizando el joystick
**Para** poder monitorizar, calibrar y automatizar el control de la luminosidad del recinto según sea necesario.

---

## Criterios de Aceptación

### CA1: Estado Inicial tras Reinicio (Reset)
* **Dado que** el sistema de iluminación acaba de recibir un reinicio (reset).
* **Cuando** el sistema completa su secuencia de arranque.
* [cite_start]**Entonces** el sistema debe iniciar obligatoriamente en el modo REPOSO[cite: 63].
* [cite_start]**Y** la pantalla LCD debe mostrar el texto "SBM 2026" junto con la hora inicial configurada a las "09:50:00"[cite: 65, 67].
* [cite_start]**Y** el LED RGB VERDE de la tarjeta mbed APPBoard debe encenderse permanentemente realizando un barrido continuo de luminosidad del 10% al 90% y viceversa, con una duración de dos segundos por barrido[cite: 69, 73].

### CA2: Transición de REPOSO a MANUAL
* **Dado que** el sistema se encuentra actualmente en modo REPOSO.
* [cite_start]**Cuando** el usuario realiza una pulsación larga (duración mayor a 1 segundo) en el botón CENTER del joystick[cite: 62].
* [cite_start]**Entonces** el sistema debe transicionar al modo MANUAL[cite: 66].
* [cite_start]**Y** el LCD debe actualizarse para mostrar el texto indicativo del modo "M-PWM", el porcentaje del ciclo de trabajo, la hora actual, y las barras gráficas verticales correspondientes a la luminosidad medida (Lmed) y la de referencia (Lref)[cite: 80, 81, 82].

### CA3: Transición de MANUAL a AUTO
* **Dado que** el sistema se encuentra operando en el modo MANUAL.
* [cite_start]**Cuando** el usuario realiza una pulsación larga en la posición CENTER del joystick[cite: 86].
* [cite_start]**Entonces** el sistema debe transicionar al modo AUTO[cite: 86].
* [cite_start]**Y** la pantalla LCD debe cambiar su indicador de modo superior para mostrar "A-PWM" manteniendo la visualización de la hora y las barras de Lmed y Lref[cite: 91, 98].

### CA4: Transición de AUTO a PROGRAMACIÓN
* **Dado que** el sistema está operando en el modo AUTO.
* [cite_start]**Cuando** el usuario efectúa una pulsación larga en la posición CENTER del joystick[cite: 104].
* [cite_start]**Entonces** el sistema debe cambiar al modo PROGRAMACIÓN[cite: 104].
* [cite_start]**Y** la pantalla LCD debe limpiar la vista anterior y mostrar la palabra "PROGRAMACION" en la línea superior, seguida de la hora ("H") y la luminosidad de referencia ("Lr") en la línea inferior[cite: 108, 109].

### CA5: Transición de PROGRAMACIÓN a REPOSO
* **Dado que** el sistema se encuentra en el modo PROGRAMACIÓN.
* [cite_start]**Cuando** el usuario ejecuta una pulsación larga en la posición CENTER del joystick[cite: 113].
* [cite_start]**Entonces** el sistema debe abandonar el modo de programación y volver al modo REPOSO[cite: 113].
* [cite_start]**Y** la pantalla LCD debe volver a mostrar la pantalla inicial con el texto "SBM 2026" y la hora del sistema[cite: 65].