#include "stm32f10x_it.h"
#include "FreeRTOS.h"
#include "task.h"

// FreeRTOS 移植：SVC/PendSV/SysTick 已由 port.c 接管
// 启动文件向量表指向 vPortSVCHandler / xPortPendSVHandler / xPortSysTickHandler

void NMI_Handler(void) { }
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void DebugMon_Handler(void) { }

// FreeRTOS 钩子函数
void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    while (1);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    while (1);
}
