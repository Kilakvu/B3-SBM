#include <stdio.h>
#include "USART.h"

/* Minimal retarget of printf to the project's USART
 * Keil ARMCC uses fputc for printf redirection.
 */
int fputc(int ch, FILE *f) {
    uint8_t c = (uint8_t)ch;
    // send character (non-blocking may be implemented inside tx_USART)
    tx_USART(&c, 1);
    return ch;
}

