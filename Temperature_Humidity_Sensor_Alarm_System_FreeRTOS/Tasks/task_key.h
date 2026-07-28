#ifndef __TASK_KEY_H
#define __TASK_KEY_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// 按键任务函数
void vTask_Key(void* pvParameters);
extern QueueHandle_t keyQueue;   // 在 main.c 中定义

#endif
