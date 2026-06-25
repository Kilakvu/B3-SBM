/**
  ******************************************************************************
  * @file    Templates/Src/RTC.c 
  * @author  MCD Application Team
  * @brief   Fichero de inicialización y configuración del RTC con el formato 24 horas.
  ******************************************************************************
  */

#include "RTC.h"
#include "mktime.h"
#include "cmsis_os2.h"

RTC_HandleTypeDef hrtc;
RTC_DateTypeDef sDate;
RTC_TimeTypeDef sTime;

#define RTCCIF  0
#define RTCALF  1

/**
  * @brief Función que resetea la hora del RTC y establece las 00:00:00 del 01/01/2020
  */
void resetHoraFecha0(void){
    /* Set RTC to 00:00:00 01/01/2020 */
    setHora(0, 0, 0);
    setFecha(1, 1, 2020);
}

/**
  * @brief Función que establece la hora del RTC con los parametros que se pasan a la función
  */
void setHora(uint8_t seg, uint8_t min, uint8_t hor){
    sTime.Hours = hor; // set hours
    sTime.Minutes = min; // set minutes
    sTime.Seconds = seg; // set seconds
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
       printf("Fallo al establecer la hora");
    }

    /* Wait for RTC to exit Init Mode and synchronize */
    osDelay(10);

    /* Force synchronization by reading backup register */
    uint32_t dummy = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
    (void)dummy;
}

/**
  * @brief Función que establece la fecha del RTC con los parametros que se pasan a la función
  */
void setFecha(uint8_t dia, uint8_t mes, uint16_t anio ){
    sDate.Month = mes; // month
    sDate.Date = dia; // date
    sDate.Year = anio; // year

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
       printf("Fallo al establecer la fecha");
    }

    /* Wait for RTC to exit Init Mode and synchronize */
    osDelay(10);

    /* Force synchronization by reading backup register */
    uint32_t dummy = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
    (void)dummy;
}

/**
  * @brief Función que obtiene el valor de los segundos del RTC
  */
uint8_t getSeg(void){
    // CORREGIDO: Leer Time bloquea los registros sombra, leer Date los desbloquea.
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sTime.Seconds;
}

/**
  * @brief Función que obtiene el valor de los minutos del RTC
  */
uint8_t getMin(void){
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sTime.Minutes;
}

/**
  * @brief Función que obtiene el valor de la hora del RTC
  */
uint8_t getHora(void){
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sTime.Hours;
}

/**
  * @brief Función que obtiene el día del RTC
  */
uint8_t getDia(void){
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sDate.Date;
}

/**
  * @brief Función que obtiene el mes del RTC
  */
uint8_t getMes(void){
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sDate.Month;
}

/**
  * @brief Función que obtiene el año del RTC
  */
uint16_t getAnio(void){
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    return sDate.Year;
}

/**
  * @brief Función que obtiene el valor de los segundos pasados desde el 1 de Enero de 1970
  */
uint64_t getTotalSeconds(void){
    uint64_t segundos;
    segundos = unix_time_in_seconds(getSeg(), getMin(), getHora(), getDia(), getMes(), getAnio());
    return segundos;
}

/**
  * @brief Función que inicializa el RTC con el formato 24h
  */
void MX_RTC_Init(void)
{
  /* Check which clock source is configured and set predivisors accordingly */
  uint32_t rtc_sel = (RCC->BDCR & RCC_BDCR_RTCSEL) >> RCC_BDCR_RTCSEL_Pos;
  uint32_t asynch_prediv = 127;  // Default for LSE
  uint32_t synch_prediv = 255;   // Default for LSE

  printf("\n=== RTC Initialization ===\n");
  printf("RTC clock source selection bits: 0x%lx\n", rtc_sel);

  /* Adjust predivisors based on clock source */
  /* Using LSI: frequency ~32 kHz */
  if (rtc_sel == 2) {
    asynch_prediv = 127;
    synch_prediv = 249;  /* LSI: 32000 / ((127+1) * (249+1)) = 1 Hz */
    printf("RTC: Using LSI (32 kHz) - AsynchPrediv=127, SynchPrediv=249\n");
  } else if (rtc_sel == 1) {
    /* LSE clock source (32.768 kHz) */
    asynch_prediv = 127;
    synch_prediv = 255;  /* LSE: 32768 / ((127+1) * (255+1)) = 1 Hz */
    printf("RTC: Using LSE (32.768 kHz) - AsynchPrediv=127, SynchPrediv=255\n");
  } else if (rtc_sel == 3) {
    /* HSE/128 clock source (fast) */
    asynch_prediv = 99;
    synch_prediv = 99;
    printf("RTC: Using HSE/128 - AsynchPrediv=99, SynchPrediv=99\n");
  } else {
    printf("ERROR: RTC clock source NOT configured! (BDCR=0x%lx)\n", RCC->BDCR);
    return;
  }

  /* Initialize RTC */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = asynch_prediv;
  hrtc.Init.SynchPrediv = synch_prediv;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  printf("Calling HAL_RTC_Init() with AsynchPrediv=%lu, SynchPrediv=%lu\n", asynch_prediv, synch_prediv);

  if (HAL_RTC_Init(&hrtc) != HAL_OK) {
    printf("ERROR: HAL_RTC_Init() failed!\n");
    return;
  }

  printf("SUCCESS: HAL_RTC_Init() completed\n");

  /* Allow RTC to exit initialization mode properly */
  osDelay(50);

  /* Verify RTC is working by reading RSF (Registers Synchronized Flag) */
  uint32_t max_wait = 1000;
  while ((RTC->ISR & RTC_ISR_RSF) == 0 && max_wait--) {
    osDelay(1);
  }

  if (max_wait == 0) {
    printf("WARNING: RTC Registers Synchronized Flag timeout\n");
  } else {
    printf("RTC: Registers synchronized successfully\n");
  }

  /* Enable Calibration Output (1 Hz) */
  HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ);
  printf("RTC: Calibration output enabled\n");

  /* Check if RTC was already configured by checking backup register */
  uint32_t bk = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
  printf("RTC: Backup register DR0 = 0x%lx\n", bk);

  if (bk != 0x32F2) {
    /* First boot - initialize time */
    printf("RTC: First boot detected - initializing time to 09:50:00 01/01/2020\n");
    setHora(0, 50, 9);    /* seg=0, min=50, hor=9 */
    setFecha(1, 1, 2020); /* día=1, mes=1, año=2020 */
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
    printf("RTC: Initial time set and backup marker written\n");

    /* Give RTC time to stabilize after initial write (reduced delay) */
    osDelay(50);
    printf("RTC: Initialization delay complete\n");
  } else {
    /* Already configured - read and verify current time */
    osDelay(50);
    uint8_t hh = getHora();
    uint8_t mm = getMin();
    uint8_t ss = getSeg();
    printf("RTC: Already configured. Current time: %02d:%02d:%02d\n", hh, mm, ss);
  }

  printf("=== RTC Initialization Complete ===\n\n");
}

// Monitor thread: detects if RTC seconds stop advancing and attempts recovery
static void RTC_Monitor_Thread(void *arg) {
    (void)arg;
    uint8_t prev = getSeg();
    int stuck_count = 0;

    for (;;) {
        osDelay(1000);
        uint8_t cur = getSeg();

        if (cur == prev) {
            stuck_count++;
        } else {
            stuck_count = 0;
        }

        prev = cur;

        if (stuck_count >= 3) {
            printf("RTC Monitor: detected RTC stuck at %02d, attempting recovery using LSI\n", cur);
            // Attempt to force RTC to use LSI and reset time
            __HAL_RCC_PWR_CLK_ENABLE();
            HAL_PWR_EnableBkUpAccess();

            // Reset backup domain
            __HAL_RCC_BACKUPRESET_FORCE();
            __HAL_RCC_BACKUPRESET_RELEASE();

            // Configure LSI oscillator
            RCC_OscInitTypeDef RCC_OscInitStructLSI = {0};
            RCC_OscInitStructLSI.OscillatorType = RCC_OSCILLATORTYPE_LSI;
            RCC_OscInitStructLSI.LSIState = RCC_LSI_ON;
            RCC_OscInitStructLSI.PLL.PLLState = RCC_PLL_NONE;

            if (HAL_RCC_OscConfig(&RCC_OscInitStructLSI) == HAL_OK) {
                RCC_PeriphCLKInitTypeDef PeriphClkInitStructLSI = {0};
                PeriphClkInitStructLSI.PeriphClockSelection = RCC_PERIPHCLK_RTC;
                PeriphClkInitStructLSI.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

                if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStructLSI) == HAL_OK) {
                    __HAL_RCC_RTC_ENABLE();

                    // Re-initialize RTC instance
                    HAL_RTC_DeInit(&hrtc);
                    hrtc.Instance = RTC;
                    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
                    hrtc.Init.AsynchPrediv = 127;
                    hrtc.Init.SynchPrediv = 255;
                    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
                    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
                    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

                    if (HAL_RTC_Init(&hrtc) == HAL_OK) {
                        setHora(0, 50, 9);
                        setFecha(1, 1, 2020);
                        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
                        printf("RTC Monitor: recovery done, RTC set to 09:50:00 using LSI\n");
                    } else {
                        printf("RTC Monitor: HAL_RTC_Init failed after forcing LSI\n");
                    }
                } else {
                    printf("RTC Monitor: HAL_RCCEx_PeriphCLKConfig failed for LSI\n");
                }
            } else {
                printf("RTC Monitor: HAL_RCC_OscConfig failed to start LSI\n");
            }
            // reset counter and continue monitoring
            stuck_count = 0;
        }
    }
}

void RTC_StartMonitor(void) {
    const osThreadAttr_t attr = { .name = "rtc_mon", .stack_size = 512 };
    osThreadNew(RTC_Monitor_Thread, NULL, &attr);
}