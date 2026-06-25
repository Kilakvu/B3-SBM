#include "USART.h"
#include "stm32f4xx.h" 
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>

extern ARM_DRIVER_USART Driver_USART3;
static ARM_DRIVER_USART * USARTdrv = &Driver_USART3;

static osMessageQueueId_t mid_UART_Frames;

void USART_Callback(uint32_t event) {
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        // Handle in thread
    }
}

int init_USART(void) {
    USARTdrv->Initialize(USART_Callback);
    USARTdrv->PowerControl(ARM_POWER_FULL);
    USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                      ARM_USART_DATA_BITS_8 |
                      ARM_USART_PARITY_NONE |
                      ARM_USART_STOP_BITS_1 |
                      ARM_USART_FLOW_CONTROL_NONE, 9600);
    USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control(ARM_USART_CONTROL_RX, 1);

    mid_UART_Frames = osMessageQueueNew(4, sizeof(UART_Frame), NULL);
    return 0;
}

int tx_USART(uint8_t ch[], int size) {
    USARTdrv->Send(ch, size);
    while(USARTdrv->GetStatus().tx_busy);
    return 0;
}

void USART_SendResponse(uint8_t cmd, const char *payload) {
    uint8_t frame[128];
    uint8_t payload_len = payload ? strlen(payload) : 0;
    uint8_t total_len = payload_len + 4; // SOH, CMD, LEN, ..., EOT

    frame[0] = SOH;
    frame[1] = cmd;
    frame[2] = total_len;
    if (payload) memcpy(&frame[3], payload, payload_len);
    frame[total_len - 1] = EOT;

    tx_USART(frame, total_len);
}

// Thread to handle reception and framing
void USART_Thread(void *argument) {
    uint8_t c;
    uint8_t state = 0;
    UART_Frame current_frame;
    uint8_t payload_idx = 0;

    while (1) {
        USARTdrv->Receive(&c, 1);
        while(USARTdrv->GetStatus().rx_busy) osDelay(1);
        
        switch(state) {
            case 0: // Wait for SOH
                if (c == SOH) state = 1;
                break;
            case 1: // CMD
                current_frame.cmd = c;
                state = 2;
                break;
            case 2: // LEN
                current_frame.len = c;
                if (current_frame.len > 4) {
                    state = 3;
                    payload_idx = 0;
                } else if (current_frame.len == 4) {
                    state = 4; // No payload
                } else {
                    state = 0; // Error
                }
                break;
            case 3: // Payload
                current_frame.payload[payload_idx++] = c;
                if (payload_idx >= (current_frame.len - 4)) {
                    state = 4;
                }
                break;
            case 4: // EOT
                if (c == EOT) {
                    current_frame.payload[payload_idx] = '\0'; // Null terminate
                    osMessageQueuePut(mid_UART_Frames, &current_frame, 0, 0);
                }
                state = 0;
                break;
        }
    }
}

int USART_GetFrame(UART_Frame *frame) {
    if (osMessageQueueGet(mid_UART_Frames, frame, NULL, 0) == osOK) {
        return 1;
    }
    return 0;
}
