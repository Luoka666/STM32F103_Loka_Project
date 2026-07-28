#ifndef __TASK_STATEMACHINE_H
#define __TASK_STATEMACHINE_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// 状态枚举（直接从裸机版搬过来用）
typedef enum {
    STOP = 0,
    RUN,
    SETTING_MENU,
    SETTING_HISTORY,
    SETTING_CHANGE,
    SETTING_CHANGE_TEMP,
    SETTING_CHANGE_HUMI
} SystemState;

// 按键宏定义（从裸机版直接搬过来）
#define KEY_RUN_STOP       1
#define KEY_CONFIRM        2
#define KEY_UP             3
#define KEY_DOWN           4
#define KEY_SETTING_BACK   5

// 全局变量声明
extern SystemState currentState;
extern uint8_t menu_index;
extern uint8_t threshold_menu_index;
extern uint8_t temp_threshold;
extern uint8_t humi_threshold;

// 互斥锁声明
extern SemaphoreHandle_t oledMutex;

// 状态机任务函数
void vTask_StateMachine(void* pvParameters);

#endif
