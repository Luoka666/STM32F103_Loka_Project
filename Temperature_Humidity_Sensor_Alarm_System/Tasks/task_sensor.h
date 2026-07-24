#ifndef __TASK_SENSOR_H
#define __TASK_SENSOR_H

#include "FreeRTOS.h"
#include "queue.h"

// 传感器数据结构体
typedef struct {
    uint8_t temperature;
    uint8_t humidity;
} SensorData_t;

// 队列句柄（在 main.c 中定义，这里在外部声明）
extern QueueHandle_t sensorQueue;

// 传感器任务函数
void vTask_Sensor(void* pvParameters);

#endif
