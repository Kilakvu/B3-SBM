#ifndef __USART_H
#define __USART_H

#include "Driver_USART.h"

#define SOH 0x01
#define EOT 0xFE

// Commands from PC
#define CMD_SET_TIME    0x20
#define CMD_SET_LREF    0x25
#define CMD_READ_MEAS   0x55
#define CMD_CLEAR_MEAS  0x60

// Responses from System
#define RESP_TIME_OK    0xDF
#define RESP_LREF_OK    0xDA
#define RESP_MEASURE    0xAF
#define RESP_CLEAR_OK   0x9F

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[64];
} UART_Frame;

int init_USART(void);
int tx_USART(uint8_t ch[], int size);
int USART_GetFrame(UART_Frame *frame);
void USART_SendResponse(uint8_t cmd, const char *payload);
void USART_Thread(void *argument);

#endif
