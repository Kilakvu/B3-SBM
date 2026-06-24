/*------------------------------------------------
*ARCHIVO THREAD: CODIFICA LAS TAREAS DEL SISTEMA
ALUMNO: RICARDO GÓMEZ MATEOS
VERSION: FINAL
FECHA: 18/12/2020
*----------------------------------------------*/
#include "cmsis_os.h"  
#include "Driver_I2C.h"
#include "joystick.h"
#include "define.h"
#include "rtc.h"
RTCTime local_time, alarm_time, current_time;
/* 							DRIVER USART0				*/
extern ARM_DRIVER_USART Driver_USART0;							
static ARM_DRIVER_USART *USARTdrv = &Driver_USART0;
/*							DRIVER I2C				*/
extern ARM_DRIVER_I2C   Driver_I2C2;
static ARM_DRIVER_I2C   *I2Cdev = &Driver_I2C2;
/*---------------------------------------------------------
*			Mide la temperatura del sistema										
*			Return temperatura tipo float
*----------------------------------------------------------*/
float medir_temp (void){
	data_write[0]=LM75_REG_TEMP;
  I2Cdev->MasterTransmit(LM75B_I2C_ADDR,data_write,1,true);
  while(I2Cdev->GetStatus().busy);
  I2Cdev->MasterReceive(LM75B_I2C_ADDR,data_read,2,false);
  while(I2Cdev->GetStatus().busy);
	i16 =(data_read[0] <<8) | data_read[1];
	temp = i16 / 256.0;
	
	return temp;
}
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
void EINT3_IRQHandler (void);
void Estado_Normal (void const *argument);                             
osThreadId tid_Estado_Normal;                                          
osThreadDef (Estado_Normal, osPriorityNormal, 1, 1500);

void Estado_Test (void const *argument);                             
osThreadId tid_Estado_Test;                                          
osThreadDef (Estado_Test, osPriorityNormal, 1, 1500);

void Estado_Desarmado (void const *argument);                             
osThreadId tid_Estado_Desarmado;                                          
osThreadDef (Estado_Desarmado, osPriorityNormal, 1, 1500); 

/*-------------THREAD PARA FLANCO DE SUBIDA---------------------*/
void Subida (void const *argument);                             // thread function
osThreadId tid_subida;                                          // thread id
osThreadDef (Subida, osPriorityNormal, 1, 0);                   // thread object
/*---------------------------------------------------------------*/
/*-------------THREAD PARA FLANCO DE BAJADA---------------------*/
void Bajada (void const *argument);                             // thread function
osThreadId tid_bajada;                                          // thread id
osThreadDef (Bajada, osPriorityNormal, 1, 0);                   // thread object
/*---------------------------------------------------------------*/
/*--------------- PERIODIC Timer Example -------------------*/
static void TIMER1 (void const *arg);                  // prototype for timer callback function
static osTimerId id_timer1;                                           // timer id
static uint32_t  exec3;                                         // argument for the timer call back function
static osTimerDef (periodic, TIMER1); 
/*-----------------------------------------------------*/

/*---------------- One-Shoot Timer Example ----------------------*/
static void Timer1_Callback (void const *arg);                  	// prototype for timer callback function
static osTimerId id1;                                           	// timer id
static uint32_t  exec1;                                         	// argument for the timer call back function
static osTimerDef (one_shot, Timer1_Callback);                   // define timers
/*-----------------One-Shoot Timer Function----------------------*/

/*---------------- One-Shoot Timer Example ----------------------*/
static void Timer2_Callback (void const *arg);                  	// prototype for timer callback function
static osTimerId id2;                                           	// timer id
static uint32_t  exec2;                                         	// argument for the timer call back function
static osTimerDef (one_shot2, Timer2_Callback);

/*---------------------------------------------------------
*			Inicializacion de los threads, I2C, RTC y USART								
*			
*----------------------------------------------------------*/
int Init_Thread (void) {
	RTCInit();
	
  //RTCSetTime( local_time );		
  NVIC_EnableIRQ(RTC_IRQn);
 
  RTCStart();
	/* Set local time */
  local_time.RTC_Sec = 0;
  local_time.RTC_Min = 0;
  local_time.RTC_Hour = 0;
	init();
  lcd_reset();
	exec3 = 1;
	id_timer1 = osTimerCreate (osTimer(periodic), osTimerPeriodic , &exec3);
	
  id1 = osTimerCreate (osTimer(one_shot), osTimerOnce, &exec1);
	id2 = osTimerCreate (osTimer(one_shot2), osTimerOnce, &exec2);
  tid_Estado_Normal = osThreadCreate (osThread(Estado_Normal), NULL);
	tid_Estado_Test = osThreadCreate (osThread(Estado_Test), NULL);
	tid_Estado_Desarmado = osThreadCreate (osThread(Estado_Desarmado), NULL);
	tid_subida = osThreadCreate (osThread(Subida), NULL);
	tid_bajada = osThreadCreate (osThread(Bajada), NULL);
	osTimerStart (id_timer1, 1000);

  if ((!tid_Estado_Normal)||(!tid_Estado_Test)) return(-1);
  	/*************************************************************************************************
                           I2CDEV INICIALIZACION [TEMP Y MMA7660]
*************************************************************************************************/
	I2Cdev->Initialize (NULL);
  I2Cdev->PowerControl(ARM_POWER_FULL);
  I2Cdev->Control(ARM_I2C_BUS_SPEED,ARM_I2C_BUS_SPEED_FAST);
  I2Cdev->Control(ARM_I2C_BUS_CLEAR,0);
	/*************************************************************************************************
                           UART INICIALIZACION
*************************************************************************************************/
	USARTdrv->Initialize(NULL);
	USARTdrv->PowerControl(ARM_POWER_FULL);
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 9600);
	 USARTdrv->Control (ARM_USART_CONTROL_TX, 1); 
	 
	/*************************************************************************************************
                           TEMP_REG_CONF
*************************************************************************************************/
	data_write[0]=LM75_REG_CONF;
  data_write[1]=0x02;
  I2Cdev->MasterTransmit(LM75B_I2C_ADDR,data_write,2,false);
  while(I2Cdev->GetStatus().busy);
	/*************************************************************************************************
                           MM17660 CONF
*************************************************************************************************/
	data_write[0] = REG_MODE;
	data_write[1]= 0x01;
	I2Cdev->MasterTransmit(MMA7660_I2C_ADDR,data_write,2,false);
  while(I2Cdev->GetStatus().busy);
  return(0);
}
/*---------------------------------------------------------
*			TIMER PERIODICO							 1s			
*			Realiza las operaciones de medir temperatura 
*			Lectura de osciloscopio y RTC
*----------------------------------------------------------*/
void TIMER1(void const *argument){
	 current_time = RTCGetTime();	
		hour = current_time.RTC_Hour;
		min  = current_time.RTC_Min; 
		sec  = current_time.RTC_Sec;  
		medir_temp();
	data_write[0] = REG_TILT;
	I2Cdev->MasterTransmit(MMA7660_I2C_ADDR,data_write,1,true);
  while(I2Cdev->GetStatus().busy);
	I2Cdev->MasterReceive(MMA7660_I2C_ADDR,data_read,1,false);
  while(I2Cdev->GetStatus().busy);	
	PoLa = (data_read[0] << 3) >>5;
	
	if(flagEstado==NORMAL)
	{
		osSignalSet(tid_Estado_Normal,0x0001);
	}
	if(flagEstado==TEST)
	{
		Desactivar_Alarma();
		osSignalSet(tid_Estado_Test,0x0001);
	}
	if(flagEstado==DESARMADO)
	{
		osSignalSet(tid_Estado_Desarmado,0x0001);
	}
	
	
  
}
/*---------------------------------------------------------
*							ESTADO NORMAL 									
*----------------------------------------------------------*/

void Estado_Normal (void const *argument) {
osEvent evt;
  while (1) {
    evt= osSignalWait(0x0001,osWaitForever);
		if(evt.value.signals == 0x0001){
			
			if ((temp>=(tmepRef-guarda))&&(temp<=tmepRef+guarda))		// VALOR DE ALARMA OFF: NO SE SUPERA LA TEMPERATURA
		{
			Desactivar_Alarma();
			RGB_VERDE();
			clear_lcd();
			sprintf(mensajeUART,"%s->%.2d:%.2d:%.2d ->Tm:%.2f->Tr:%.2f->Th:%.2f \r\n",estado,hour,min,sec,temp,tmepRef,guarda);
			USARTdrv->Send(mensajeUART, 60);
			sprintf(mensaje, "%s->%.2d:%.2d:%.2d",estado,hour,min,sec);
			EscribeFrase(mensaje,1);
			sprintf(mensaje, "Tm:%.3f Tr:%.1f Th:%.1f",temp,tmepRef,guarda);
			EscribeFrase(mensaje,2);
		}
		else{																												// VALOR DE ALARMA ON: SE SUPERA LA TEMPERATURA
			Activar_Alarma();
			RGB_ROJO();
			clear_lcd();
			sprintf(mensajeUART,"%s->%.2d:%.2d:%.2d ->Tm:%.2f->Tr:%.2f->Th:%.2f \r\n",estado,hour,min,sec,temp,tmepRef,guarda);
			USARTdrv->Send(mensajeUART, 60);
			sprintf(mensaje, "%s->%.2d:%.2d:%.2d",estado,hour,min,sec);
			EscribeFrase(mensaje,1);
			sprintf(mensaje, "Tm:%.3f Tr:%.1f Th:%.1f",temp,tmepRef,guarda);
			EscribeFrase(mensaje,2);
			if(PoLa==2)
			{
				flagEstado=DESARMADO;	//PASA A ESTADO DESARMADO
				horaDes=hour;
				minDes=min;
				secDes=sec;
				sprintf(estado,"DESARMADO");
				
			}
		}

			osThreadYield ();  
		}			
  }
}
/*---------------------------------------------------------
*							ESTADO TEST 									
*----------------------------------------------------------*/

void Estado_Test (void const *argument) {
osEvent evt;
  while (1) {
    evt= osSignalWait(0x0001,osWaitForever);
		if(evt.value.signals == 0x0001){
			switch(PoLa){
			case 1:						/*POSICION UP*/
			RGB_ROJO();
			clear_lcd();
			sprintf(mensajeUART,"%s ->%.2d:%.2d:%.2d ->Tm:%.2f\r\r\r\n",estado,hour,min,sec,temp);
			USARTdrv->Send(mensajeUART, 35);
			sprintf(mensaje,"%s Tm:%.3f",estado,temp);
			EscribeFrase(mensaje,1);
			
			break;
			case 5:			/*POSICION LEFT*/
						
			clear_lcd();
			sprintf(mensajeUART,"%s ->%.2d:%.2d:%.2d ->RGB\r\r\r\n",estado,hour,min,sec);
			USARTdrv->Send(mensajeUART, 35);
			EscribirTEST(letraTest[1]); //L
			
			RGB_VERDE();
			retardo_ms(500);
			RGB_ROJO();
			retardo_ms(500);
			RGB_AZUL();
			retardo_ms(500);
			
			break;
			case 2:					/* POSICION ABAJO */
				
				RGB_ROJO();
				clear_lcd();
				sprintf(mensajeUART,"%s ->%.2d:%.2d:%.2d ->EXTENDED\r\r\r\n",estado,hour,min,sec);
				USARTdrv->Send(mensajeUART, 35);
				EscribirTEST(letraTest[2]);	//F
			break;
			case 6: 		/*POSICION DERECHA*/
				Activar_Alarma();
				RGB_ROJO();
				clear_lcd();
				sprintf(mensajeUART,"%s ->%.2d:%.2d:%.2d ->BUZZER\r\r\r\n",estado,hour,min,sec);
				USARTdrv->Send(mensajeUART, 35);
				EscribirTEST(letraTest[0]); //R
				Desactivar_Alarma();
			break;
			default: break;
		}
			
			
			
    osThreadYield ();  
		}			
  }
}
/*---------------------------------------------------------
*							ESTADO DESARMADO						
*----------------------------------------------------------*/
void Estado_Desarmado (void const *argument) {
osEvent evt;
  while (1) {
    evt= osSignalWait(0x0001,osWaitForever);
		if(evt.value.signals == 0x0001){
			Desactivar_Alarma();
			clear_lcd();
			sprintf(mensajeUART,"%s ->Hd:%.2d:%.2d:%.2d ->Tr:%.2f -> Th:%.2f \r\n",estado,horaDes,minDes,secDes,tmepRef,guarda);
			USARTdrv->Send(mensajeUART, 60);
			sprintf(mensaje, "%s->%.2d:%.2d:%.2d",estado,horaDes,minDes,secDes);
			EscribeFrase(mensaje,1);
			sprintf(mensaje,"TempRef:%.1f  Th:%.1f",tmepRef,guarda);
			EscribeFrase(mensaje,2);
			RGB_AZUL();
			
	osThreadYield ();  
		}
	}
}
void Timer1_Callback (void const *arg)
{
	/*HABILITAMOS LAS INTERRUPCIONES DE BAJADA*/
	LPC_GPIOINT->IO0IntEnF |= ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
	
}
void Timer2_Callback (void const *arg)
{
	flag_joystick();
	/*HABILITAMOS LAS INTERRUPCIONES DE SUBIDA*/
	LPC_GPIOINT->IO0IntEnR |= ((1 << ARRIBA) | (1 << ABAJO) | (1 << CENTRO) | (1 << DERECHA) | (1 << IZQUIERDA));
	
}
void Subida (void const *argument) {

  while (1) {
    osSignalWait(SUBIDA,osWaitForever);
		osTimerStart(id1,20);	//20ms para el rebote
    osThreadYield ();                                           // suspend thread
  }
}
void Bajada (void const *argument) {

  while (1) {
    osSignalWait(BAJADA,osWaitForever);
		osTimerStart(id2,20); //20ms para el rebote
    osThreadYield ();                                           // suspend thread
  }
}
