#ifndef HOST_TASK_H
#define HOST_TASK_H
#include "FreeRTOS.h"
#define taskSCHEDULER_NOT_STARTED 0
#define taskSCHEDULER_RUNNING 2
int xTaskGetSchedulerState(void);
void vTaskDelay(TickType_t ticks);
#endif
