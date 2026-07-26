#ifndef __TASK_ALARM_H
#define __TASK_ALARM_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 报警任务函数
void vTask_Alarm(void* pvParameters);

#endif
