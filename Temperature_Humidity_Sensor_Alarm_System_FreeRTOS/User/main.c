#include "FreeRTOS.h" // 放在所有头文件之前
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
#include "timer_delay.h"
#include "buzzer.h"
// FreeRTOS头文件
#include "task.h"           // 任务相关所需 API（xTaskCreate、vTaskStartScheduler）
#include "queue.h"          // 队列相关所需 API（xQueueCreate、xQueueSend）
#include "stm32f10x_rcc.h"
#include "task_sensor.h"    // 传感器任务声明（SensorData_t、sensorQueue、vTask_Sensor）
#include "task_display.h"
#include "task_alarm.h"
#include "task_key.h"
#include "task_statemachine.h"
#include "task_record.h"
#include "timers.h"  // 软件定时器头文件，使用软件定时器对按键进行非阻塞消抖，之前的while()循环会干扰FreeRTOS运行。


//变量定义
uint8_t temperature = 0, humidity = 0;
SystemState currentState = STOP;
uint8_t menu_index = 0;
uint8_t threshold_menu_index = 0;

//初始报警阈值
uint8_t temp_threshold = 40, humi_threshold = 60; // 报警阈值

SemaphoreHandle_t oledMutex;
TimerHandle_t keyTimer;  // 按键扫描定时器句柄
QueueHandle_t sensorQueue; // 创建 struct QueueDefinition * 类型的指针变量，定义一个结构体指针变量 sensorQueue
QueueHandle_t alarmQueue;
QueueHandle_t keyQueue;
QueueHandle_t recordQueue;

// 在 main 函数里，硬件初始化之后，创建队列和任务
int main(void) {
    
    // 初始化外设（外设不工作，一定要先看有没有初始化😭😭😭😭）
    OLED_Init();
    DHT11_Init();
    usart_Init();
    Key_Init(); // 按键初始化（之前修好的那个）
	TIM2_Delay_Init();  // 初始化硬件定时器延时
    init_alarm(); 
	Buzzer_off(); // 初始化为低电平，防止系统开始时就鸣叫
    OLED_Clear();
	
	stop_ui();// 开机后主动画一次 STOP 界面，否则oled会显示黑屏
	
	// 创建传感器数据队列（5 个槽位） 
    sensorQueue = xQueueCreate(5, sizeof(SensorData_t));
	// 创建报警队列（5 个槽位）
	alarmQueue = xQueueCreate(5, sizeof(SensorData_t));
	// 创建按键队列（深度 5，每个元素是 uint8_t）
	keyQueue = xQueueCreate(5, sizeof(uint8_t));
	// 互斥锁，保护oled每次只能被一个任务调用
	oledMutex = xSemaphoreCreateMutex();
	// 创建历史记录队列（5 个槽位）
	recordQueue = xQueueCreate(5, sizeof(SensorData_t));
	
	/*Sensor 优先级提到 3 → 谁也不能抢它。
	Alarm 降到 2 不影响紧迫性——它阻塞在 xQueueReceive，收到数据就能立刻跑。
	Key 降到 1 → 永远在 Sensor 空闲时才调度。*/
	
    // 创建传感器采集任务（优先级 2，栈 128 字）
    xTaskCreate(vTask_Sensor, "Sensor", 128, NULL, 3, NULL); // 最高（DHT11 不能被抢）
	// 创建oled显示任务（优先级 1，栈 256 字）
	xTaskCreate(vTask_Display, "Display", 256, NULL, 1, NULL);  // 优先级 1，最低
	// 创建报警任务（优先级 3，栈 128 字）
	xTaskCreate(vTask_Alarm, "Alarm", 128, NULL, 2, NULL); // 报警任务
	// 创建按键任务（优先级 2，栈 128 字）
	xTaskCreate(vTask_Key, "Key", 128, NULL, 1, NULL);
	// 创建状态机任务（优先级 2，栈 256 字）
	xTaskCreate(vTask_StateMachine, "StateM", 256, NULL, 2, NULL);
	// 创建历史存储任务（优先级 1，最低，栈 128 字）
	xTaskCreate(vTask_Record, "Record", 128, NULL, 1, NULL);
	
	// 创建按键扫描软件定时器（周期 10ms，自动重装载）
	keyTimer = xTimerCreate(
		"KeyTimer",                    // 定时器名称
		pdMS_TO_TICKS(10),             // 周期 10ms
		pdTRUE,                        // 自动重装载（循环执行）
		NULL,                          // 定时器 ID（不用的话就填 NULL）
		vKeyTimerCallback              // 回调函数
	);
	// 启动定时器
	xTimerStart(keyTimer, 0);

	
    // 启动调度器
    vTaskStartScheduler();
    while (1);

}
