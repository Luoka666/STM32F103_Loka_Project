# FreeRTOS LED 闪烁 — 移植示例

将裸机 LED 闪烁程序改造为 FreeRTOS 双任务并发执行。

## 改动要点

### 1. FreeRTOS 接管三个中断

| 中断 | 裸机 | FreeRTOS |
|------|------|----------|
| SysTick | `Delay_ms()` 阻塞延时 | `xPortSysTickHandler()` → 系统心跳 |
| PendSV | 空函数 | `xPortPendSVHandler()` → 任务上下文切换 |
| SVC | 空函数 | `vPortSVCHandler()` → 启动第一个任务 |

### 2. 启动文件修改

`startup_stm32f10x_md.s` 中三个中断向量需改为 FreeRTOS 的函数名：

| 原函数名 | 改为 |
|----------|------|
| `SVC_Handler` | `vPortSVCHandler` |
| `PendSV_Handler` | `xPortPendSVHandler` |
| `SysTick_Handler` | `xPortSysTickHandler` |

### 3. FreeRTOSConfig.h

`configCPU_CLOCK_HZ = 72MHz`（STM32F103 主频）
`configTICK_RATE_HZ = 1000`（1ms 系统节拍）

### 4. vTaskDelay 替代 Delay_ms

裸机：`Delay_ms(500)` — 阻塞 500ms，CPU 空转

FreeRTOS：`vTaskDelay(pdMS_TO_TICKS(500))` — 当前任务挂起 500ms，CPU 运行其他任务

## Keil 工程配置

### 需添加的源文件

```
FreeRTOS/
├── Source/
│   ├── tasks.c
│   ├── queue.c
│   ├── list.c
│   ├── timers.c
│   ├── event_groups.c
│   ├── stream_buffer.c
│   ├── portable/RVDS/ARM_CM3/port.c
│   └── portable/MemMang/heap_4.c
```

### 需添加的头文件路径

```
User                        (FreeRTOSConfig.h)
FreeRTOS/Source/include     (FreeRTOS.h, task.h, queue.h ...)
FreeRTOS/Source/portable/RVDS/ARM_CM3  (portmacro.h)
```

### 编译选项

- C99 模式
- 优化等级：O0（调试）/ O2（发布）

## 运行结果

- **LED1 (PA6)**：以 1Hz 频率闪烁（亮 500ms / 灭 500ms）
- **LED2 (PA7)**：以 2.5Hz 频率闪烁（亮 200ms / 灭 200ms）
- 两个任务**并发运行**，互不阻塞

## 与裸机的关键区别

| | 裸机 | FreeRTOS |
|------|------|----------|
| 延时方式 | `Delay_ms(500)` 阻塞 CPU | `vTaskDelay()` 挂起任务 |
| CPU 利用率 | 延时期间 CPU 空转 | CPU 可运行其他任务 |
| 多任务 | while(1) 中轮询实现 | Task 独立调度 |
| 扩展性 | 增加功能需修改主循环 | 新增 Task 即可 |
