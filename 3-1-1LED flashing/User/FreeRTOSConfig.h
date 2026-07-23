#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f10x.h"

// ==================== 内核配置 ====================

#define configUSE_PREEMPTION            1       // 抢占式调度
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1  // 硬件 CLZ 指令优化
#define configUSE_TICKLESS_IDLE         0       // 不使用低功耗 tickless
#define configCPU_CLOCK_HZ              (SystemCoreClock)  // 72MHz
#define configTICK_RATE_HZ              (1000)  // 1ms 系统节拍
#define configMAX_PRIORITIES            (16)    // 优先级 0~15
#define configMINIMAL_STACK_SIZE        ((unsigned short)128)
#define configTOTAL_HEAP_SIZE           ((size_t)(10 * 1024))  // 10KB 堆
#define configMAX_TASK_NAME_LEN         (16)
#define configUSE_TRACE_FACILITY        1
#define configUSE_16_BIT_TICKS          0       // 32 位 tick
#define configIDLE_SHOULD_YIELD         1       // 空闲任务主动让出 CPU

// ==================== 内核钩子 ====================

#define configUSE_MUTEXES               1
#define configUSE_RECURSIVE_MUTEXES     0
#define configUSE_COUNTING_SEMAPHORES   1
#define configQUEUE_REGISTRY_SIZE       8
#define configUSE_QUEUE_SETS            0
#define configUSE_TIME_SLICING          1       // 同优先级时间片轮转
#define configUSE_NEWLIB_REENTRANT      0
#define configENABLE_BACKWARD_COMPATIBILITY 0

// ==================== 内存管理 ====================
// 使用 heap_4.c（最佳适配，支持碎片合并）

#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

// ==================== 钩子函数 ====================

#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configCHECK_FOR_STACK_OVERFLOW  2   // 栈溢出检测（方法 2）
#define configUSE_MALLOC_FAILED_HOOK    1

// ==================== 协程（不使用） ====================
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

// ==================== 软件定时器 ====================

#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       (2)
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    (256)

// ==================== 中断配置 ====================

// 屏蔽优先级寄存器实际使用的位数（Cortex-M3 使用高 4 位）
#define configPRIO_BITS                 4

// 最低中断优先级（FreeRTOS 可屏蔽的最高逻辑优先级）
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   0x0F
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
// 等价于 configMAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS)
// 即 5 << 4 = 0x50

#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

// ==================== 断言 ====================

#define configASSERT(x)     if ((x) == 0) { taskDISABLE_INTERRUPTS(); for (;;); }

// ==================== 运行时统计（可选） ====================

#define configGENERATE_RUN_TIME_STATS    0
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() 0

// ==================== FreeRTOS API 包含开关 ====================

#define INCLUDE_vTaskPrioritySet         1
#define INCLUDE_uxTaskPriorityGet        1
#define INCLUDE_vTaskDelete              1
#define INCLUDE_vTaskSuspend             1
#define INCLUDE_vTaskDelayUntil          1
#define INCLUDE_vTaskDelay               1
#define INCLUDE_xTaskGetSchedulerState   1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle   0
#define INCLUDE_eTaskGetState            1
#define INCLUDE_xEventGroupSetBitFromISR 1
#define INCLUDE_xTimerPendFunctionCall   1
#define INCLUDE_xQueueGetMutexHolder     1
#define INCLUDE_pcTaskGetTaskName        1

#endif
