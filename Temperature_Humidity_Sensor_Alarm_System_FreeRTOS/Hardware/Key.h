#include "timers.h"
#ifndef __KEY_H
#define __KEY_H

void Key_Init(void);
uint8_t Key_GetNum(void);
void vKeyTimerCallback(TimerHandle_t xTimer); // 回调函数

#endif
