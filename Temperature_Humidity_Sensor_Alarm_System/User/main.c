#include "stm32f10x.h"
#include "dht11.h"
#include "oled.h"
#include "delay.h"
#include "USART.h"
#include "key.h"
#include "UI.h"
#include "LED.h"
#include "alarm.h"
#include "Record_storage.h"
#include <stdio.h>
// FreeRTOS头文件
#include "FreeRTOS.h"      // FreeRTOS 基础头文件
#include "task.h"           // 任务相关所需 API（xTaskCreate、vTaskStartScheduler）
#include "queue.h"          // 队列相关所需 API（xQueueCreate、xQueueSend）
#include "task_sensor.h"    // 传感器任务声明（SensorData_t、sensorQueue、vTask_Sensor）

//变量定义
uint8_t temperature = 0, humidity = 0;
uint8_t keyNum = 0;
uint8_t menu_index = 0;
uint8_t threshold_menu_index = 0;
//初始报警阈值
uint8_t temp_threshold = 40, humi_threshold = 80; // 报警阈值

QueueHandle_t sensorQueue;

// 在 main 函数里，硬件初始化之后，创建队列和任务
int main(void) {
    
    // 初始化外设（外设不工作，一定要先看有没有初始化😭😭😭😭）
    OLED_Init();
    DHT11_Init();
    usart_Init();
    Key_Init(); // 按键初始化（之前修好的那个）
    init_alarm(); 
    OLED_Clear();
	
	// 创建传感器数据队列（5 个槽位）
    sensorQueue = xQueueCreate(5, sizeof(SensorData_t));

    // 创建传感器采集任务（优先级 2，栈 128 字）
    xTaskCreate(vTask_Sensor, "Sensor", 128, NULL, 2, NULL);

    // 启动调度器
    vTaskStartScheduler();
    while (1);

    
}
