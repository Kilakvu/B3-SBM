#include "ModeManager.h"
#include "Joystick.h"
#include "VEML7700.h"
#include "RGB.h"
#include "Board_ADC.h"
#include "LCD.h"
#include "USART.h"
#include "HighBrightLED.h"
#include "RTC.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static SystemMode current_mode = MODE_REPOSO;
static float l_med = 0;
static uint16_t l_ref = 1500;
static uint8_t pwm_val = 50;
static char buffer_circular[20][64];
static uint8_t buffer_idx = 0;
static uint8_t buffer_count = 0;
static uint32_t last_log_tick = 0;
static uint32_t start_tick = 0;
static uint32_t last_update_tick = 0;  // Control display update frequency

// --- VARIABLES PARA MODO PROGRAMACIÓN ---
static uint8_t cursor_pos = 3;  // 0:Horas, 1:Minutos, 2:Segundos, 3:Lref
static uint8_t edit_h = 0, edit_m = 0, edit_s = 0;
static uint16_t edit_lref = 1500;

void ModeManager_Init(void) {
    Joystick_Init();
    VEML7700_Init();
    initRGB();
    ADC1_Init();
    GPIO_INIT();
    LCD_reset();
    init_USART();
    HighBrightLED_Init();
    MX_RTC_Init();
    // Start RTC monitor thread to detect and recover from RTC stuck situations
    RTC_StartMonitor();
}

static void LogMeasure(void) {
    char entry[64];
    sprintf(entry, "%02d:%02d:%02d--Lmed:%06.3f--Lref:%04d--PWM:%02d%%",
            getHora(), getMin(), getSeg(), l_med, l_ref, pwm_val);
    strcpy(buffer_circular[buffer_idx], entry);
    buffer_idx = (buffer_idx + 1) % 20;
    if (buffer_count < 20) buffer_count++;
}

static void ProcessUARTCommands(void) {
    UART_Frame frame;
    if (USART_GetFrame(&frame)) {
        switch(frame.cmd) {
            case CMD_SET_TIME: {
                int h, m, s;
                if (sscanf((char*)frame.payload, "%d:%d:%d", &h, &m, &s) == 3) {
                    setHora(s, m, h);  // note: setHora(seg, min, hor)
                    USART_SendResponse(RESP_TIME_OK, (char*)frame.payload);
                }
                break;
            }
            case CMD_SET_LREF: {
                l_ref = atoi((char*)frame.payload);
                USART_SendResponse(RESP_LREF_OK, (char*)frame.payload);
                break;
            }
            case CMD_READ_MEAS: {
                int i;
                for (i = 0; i < buffer_count; i++) {
                    uint8_t idx = (buffer_idx - buffer_count + i + 20) % 20;
                    USART_SendResponse(RESP_MEASURE, buffer_circular[idx]);
                }
                break;
            }
            case CMD_CLEAR_MEAS: {
                buffer_count = 0;
                buffer_idx = 0;
                USART_SendResponse(RESP_CLEAR_OK, NULL);
                break;
            }
        }
    }
}

static void UpdateDisplay(void) {
    char lines[2][21];

    uint8_t hh = getHora();
    uint8_t mm = getMin();
    uint8_t ss = getSeg();

    // Update LCD every time ModeManager_Process is called (every 100ms)
    lcd_clean();

    switch(current_mode) {
        case MODE_REPOSO:
            sprintf(lines[0], "SBM 2026");
            sprintf(lines[1], "%02d:%02d:%02d", hh, mm, ss);
            actualizar(lines);
            break;

        case MODE_MANUAL:
            LCD_DrawBar(2, (uint8_t)(l_med * 32 / 4000));
            LCD_DrawBar(8, (uint8_t)(l_ref * 32 / 4000));
            sprintf(lines[0], "M-PWM: %d%%", pwm_val);
            sprintf(lines[1], "%02d:%02d:%02d", hh, mm, ss);
            actualizar(lines);
            break;

        case MODE_AUTO:
            LCD_DrawBar(2, (uint8_t)(l_med * 32 / 4000));
            LCD_DrawBar(8, (uint8_t)(l_ref * 32 / 4000));
            sprintf(lines[0], "A-PWM: %d%%", pwm_val);
            sprintf(lines[1], "%02d:%02d:%02d", hh, mm, ss);
            actualizar(lines);
            break;

        case MODE_PROGRAMACION:
            sprintf(lines[0], "PROGRAMACION");

            // Dibujamos corchetes [ ] envolviendo el campo activo
            if (cursor_pos == 0) {
                sprintf(lines[1], "H:[%02d]:%02d:%02d Lr:%d", edit_h, edit_m, edit_s, edit_lref);
            } else if (cursor_pos == 1) {
                sprintf(lines[1], "H:%02d:[%02d]:%02d Lr:%d", edit_h, edit_m, edit_s, edit_lref);
            } else if (cursor_pos == 2) {
                sprintf(lines[1], "H:%02d:%02d:[%02d] Lr:%d", edit_h, edit_m, edit_s, edit_lref);
            } else {
                sprintf(lines[1], "H:%02d:%02d:%02d Lr:[%d]", edit_h, edit_m, edit_s, edit_lref);
            }

            actualizar(lines);
            break;
    }
}


void ModeManager_Process(void) {
    uint32_t current_tick = osKernelGetTickCount();

    if (current_tick - last_update_tick < 100) {
        return;  // Skip this execution, not yet time for update
    }
    last_update_tick = current_tick;

    uint32_t events = Joystick_GetEvents();

    ProcessUARTCommands();

    // RTC diagnostic
    static uint32_t last_diag = 0;
    if (current_tick - last_diag >= 1000) {
        uint8_t h = getHora();
        uint8_t m = getMin();
        uint8_t s = getSeg();
        printf("DIAG RTC: %02d:%02d:%02d (kernel tick: %lu)\n", h, m, s, current_tick);
        last_diag = current_tick;
    }

    // Mode transitions (Pulsación Larga Center)
    if (events & JOY_EVENT_CENTER && events & JOY_EVENT_LONG) {
        apagar_LED_rojo(); apagar_LED_verde(); apagar_LED_azul();
        switch(current_mode) {
            case MODE_REPOSO:
                current_mode = MODE_MANUAL;
                break;
            case MODE_MANUAL:
                current_mode = MODE_AUTO;
                break;
            case MODE_AUTO:
                current_mode = MODE_PROGRAMACION;
                // Inicializar variables temporales de edición
                edit_h = getHora();
                edit_m = getMin();
                edit_s = getSeg();
                edit_lref = ((l_ref + 50) / 100) * 100;
                if (edit_lref < 100) edit_lref = 100;
                if (edit_lref > 4000) edit_lref = 4000;
                cursor_pos = 3;
                break;
            case MODE_PROGRAMACION:
                current_mode = MODE_REPOSO;
                break;
        }
    }

    l_med = VEML7700_ReadLux();

    switch(current_mode) {
        case MODE_REPOSO: {
            uint32_t elapsed;
            float intensity;
            if (start_tick == 0) start_tick = current_tick;
            elapsed = (current_tick - start_tick) % 4000;
            if (elapsed < 2000) intensity = 0.1f + (0.8f * elapsed / 2000.0f);
            else intensity = 0.9f - (0.8f * (elapsed - 2000) / 2000.0f);

            apagar_LED_rojo();
            apagar_LED_azul();
            intensidad_LED_rojo(0);
            intensidad_LED_azul(0);
            int gv = (int)(intensity * 65535);
            encender_LED_verde(gv);
            break;
        }

        case MODE_MANUAL:
            l_ref = 100 + (ADC_Read(ADC_CHANNEL_3) * 3900 / 4095);
            pwm_val = 5 + (ADC_Read(ADC_CHANNEL_10) * 90 / 4095);
            HighBrightLED_SetDutyCycle(pwm_val);

            if (fabs(l_med - l_ref) > 250) encender_LED_rojo(65535);
            else apagar_LED_rojo();

            apagar_LED_azul();
            apagar_LED_verde();
            break;

        case MODE_AUTO: {
            float error = l_ref - l_med;
            if (error > 10) pwm_val = (pwm_val < 95) ? pwm_val + 1 : 95;
            else if (error < -10) pwm_val = (pwm_val > 5) ? pwm_val - 1 : 5;
            HighBrightLED_SetDutyCycle(pwm_val);

            if (fabs(l_med - l_ref) > 250) encender_LED_azul(65535);
            else apagar_LED_azul();

            if (current_tick - last_log_tick >= 1000) {
                LogMeasure();
                last_log_tick = current_tick;
            }
            break;
        }

        case MODE_PROGRAMACION:
            // 1. Navegación Izquierda/Derecha
            if (events & JOY_EVENT_LEFT) {
                if (cursor_pos > 0) cursor_pos--;
            }
            if (events & JOY_EVENT_RIGHT) {
                if (cursor_pos < 3) cursor_pos++;
            }

            // 2. Modificación Arriba/Abajo
            if (events & JOY_EVENT_UP) {
                if (cursor_pos == 0) edit_h = (edit_h + 1) % 24;
                else if (cursor_pos == 1) edit_m = (edit_m + 1) % 60;
                else if (cursor_pos == 2) edit_s = (edit_s + 1) % 60;
                else if (cursor_pos == 3) edit_lref = (edit_lref < 4000) ? edit_lref + 100 : 4000;
            }
            if (events & JOY_EVENT_DOWN) {
                if (cursor_pos == 0) edit_h = (edit_h == 0) ? 23 : edit_h - 1;
                else if (cursor_pos == 1) edit_m = (edit_m == 0) ? 59 : edit_m - 1;
                else if (cursor_pos == 2) edit_s = (edit_s == 0) ? 59 : edit_s - 1;
                else if (cursor_pos == 3) edit_lref = (edit_lref > 100) ? edit_lref - 100 : 100;
            }

            // 3. Validación (Pulsación corta Center)
            if ((events & JOY_EVENT_CENTER) && !(events & JOY_EVENT_LONG)) {
                l_ref = edit_lref;
                setHora(edit_s, edit_m, edit_h);
            }
            break;
    }

    UpdateDisplay();
}