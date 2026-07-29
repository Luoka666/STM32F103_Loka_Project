#ifndef __TASK_RECORD_H
#define __TASK_RECORD_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 历史记录存储任务函数
void vTask_Record(void* pvParameters);

#endif
