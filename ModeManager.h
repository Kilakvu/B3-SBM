#ifndef __MODEMANAGER_H
#define __MODEMANAGER_H

#include <stdint.h>

typedef enum {
    MODE_REPOSO,
    MODE_MANUAL,
    MODE_AUTO,
    MODE_PROGRAMACION
} SystemMode;

void ModeManager_Init(void);
void ModeManager_Process(void);

#endif
