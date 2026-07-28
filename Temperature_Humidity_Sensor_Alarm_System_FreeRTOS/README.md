# 智能温湿度监测与报警系统（FreeRTOS 版）

基于 **STM32F103C8T6** 标准库 + **FreeRTOS V10.3.1** 开发的嵌入式多任务实战项目。在裸机版双层状态机架构的基础上，将传感器采集、OLED 显示、报警处理、按键扫描拆分为独立 FreeRTOS 任务，通过队列进行任务间通信。

> 本项目配套 [Python 上位机](https://github.com/Luoka666/upper_computer)，通过串口接收数据并在 PC 端绘制实时温湿度动态曲线，实现从单片机到 PC 端的完整数据闭环。
>
> 本项目同时维护两个版本：[裸机版（标准库状态机）](../Temperature_Humidity_Sensor_Alarm_System_BareMetal/) | **FreeRTOS 版（当前）**

---

## 目录

- [功能特性](#功能特性)
- [FreeRTOS 任务架构](#freertos-任务架构)
- [文件结构](#文件结构)
- [使用说明](#使用说明)
- [FreeRTOS 移植踩坑记录](#freertos-移植踩坑记录)
- [待优化方向](#待优化方向未来计划)

---

## 功能特性

裸机版全部功能 + FreeRTOS 新增特性：

- **多任务并发**：4 个独立 Task（Sensor、Display、Alarm、Key）通过队列通信
- **队列解耦**：传感器数据通过 `sensorQueue` 和 `alarmQueue` 分发给消费者
- **非阻塞延时**：`vTaskDelay()` 替代 `Delay_ms()`，CPU 不空转
- **硬件定时器延时**：TIM2 提供微秒级精确延时，保护 DHT11 通信时序
- **Tick Hook 兼容**：FreeRTOS Tick Hook 替代裸机 SysTick_Handler，保留 g_millis

---

## 架构演进过程

| 版本 | 说明 |
| :--- | :--- |
| v0.1 | 裸机 `if-else` 驱动 DHT11 与 OLED，验证底层硬件 |
| v0.2 | 引入 `SystemState` 枚举，搭建双层 `switch-case` 状态机骨架 |
| v0.3 | 补全 7 种状态的 UI 绘制函数，实现菜单光标导航 |
| v0.4 | 添加清屏优化机制，解决画面闪烁与字符残留 |
| v0.5 | 接入串口调试输出，定位按键误触发与状态横跳 Bug |
| v0.6 | 修正 GPIO 时钟与端口配置错误，按键功能恢复正常 |
| v0.7 | 引入环形缓冲区，重构历史记录存储与显示逻辑 |
| v0.8 | 集成非阻塞式 LED 报警闪烁，RUN 状态下菜单锁定 |
| v0.9 | 主循环升级为非阻塞事件驱动架构，重构延时函数保护系统心跳 |
| v1.0 | 移植 FreeRTOS：拆分为 Sensor / Display / Alarm / Key 四个任务，队列通信 |

---

## FreeRTOS 任务架构

### 任务列表

| 任务 | 优先级 | 栈大小 | 周期 | 职责 |
|------|--------|--------|------|------|
| **Sensor** | 3（最高） | 128 | 100ms | DHT11 采集，数据广播到 sensorQueue + alarmQueue |
| **Alarm** | 2 | 128 | 事件驱动 | 从 alarmQueue 取数据，判断阈值，LED+蜂鸣器报警 |
| **Display** | 1 | 256 | 事件驱动 | 从 sensorQueue 取数据，刷新 OLED 显示 |
| **Key** | 1 | 128 | 20ms 轮询 | 调用 Key_GetNum() 扫描按键，键值通过 keyQueue 发送 |

### 队列设计

| 队列 | 深度 | 数据类型 | 生产者 | 消费者 |
|------|------|----------|--------|--------|
| `sensorQueue` | 5 | `SensorData_t` | Sensor Task | Display Task |
| `alarmQueue` | 5 | `SensorData_t` | Sensor Task | Alarm Task |
| `keyQueue` | 5 | `uint8_t` | Key Task | （待接入状态机） |

### 数据流

```
DHT11 → Sensor Task → sensorQueue → Display Task → OLED
                    → alarmQueue  → Alarm  Task → LED/Buzzer

按键 → Key Task → keyQueue → （状态机消费，开发中）
```

### 优先级设计原则

- Sensor 优先级最高，DHT11 微秒级通信期间不能被其他任务抢占
- Alarm 优先级中等，数据到达时立即响应
- Display 和 Key 优先级最低且同级，时间片轮转，不干扰 Sensor

### 裸机架构保留部分

- `Hardware/` 驱动层（OLED、LED、Key、Buzzer）沿用裸机版
- `System/` 逻辑层（DHT11、USART、alarm、UI、Record_storage）沿用裸机版
- `UI.c` 中的 UI 函数继续被 Display Task 调用
- `alarm.c` 中的 `alarm_run()` 逻辑已迁移到 Alarm Task
- 启动文件栈空间从 0x400 扩大到 0x800，向量表指向 FreeRTOS 中断处理函数

---

## 文件结构

```
/Hardware                  // 硬件驱动层（与裸机版共用）
├── OLED.c / OLED.h
├── Key.c / Key.h
├── LED.c / LED.h
├── buzzer.c / buzzer.h
└── timer_delay.c / timer_delay.h   // TIM2 硬件定时器延时

/System                    // 系统逻辑层（与裸机版共用）
├── dht11.c / dht11.h
├── USART.c / USART.h
├── alarm.c / alarm.h
├── UI.c / UI.h
├── Record_storage.c / Record_storage.h
└── delay.c / delay.h

/FreeRTOS/Source           // FreeRTOS V10.3.1 内核
├── tasks.c / queue.c / list.c / timers.c ...
├── include/
└── portable/RVDS/ARM_CM3/ + MemMang/

/Tasks                     // FreeRTOS 任务层（新增）
├── task_sensor.c / .h     // 传感器采集任务
├── task_display.c / .h    // OLED 显示任务
├── task_alarm.c / .h      // 报警任务
└── task_key.c / .h        // 按键扫描任务

/User
├── main.c                 // 硬件初始化 + 队列/任务创建 + 启动调度器
├── FreeRTOSConfig.h       // FreeRTOS 内核配置（72MHz / 1ms tick / 10KB heap）
└── stm32f10x_it.c         // 中断服务（SysTick 由 FreeRTOS 接管，Tick Hook 维护 g_millis）
```

---

## 使用说明

### 硬件连接

| 外设 | STM32 引脚 | 说明 |
| :--- | :--- | :--- |
| **DHT11** | PA0 | 单总线数据引脚 |
| **OLED (I2C)** | SCL: PB8, SDA: PB9 | 0.96 寸 128x64 |
| **按键 K1** | PA1 | 运行/停止总开关 |
| **按键 K2** | PA2 | 确认/保存 |
| **按键 K3** | PA3 | 向上/递增 |
| **按键 K4** | PA4 | 向下/递减 |
| **按键 K5** | PA5 | 设置/返回 |
| **报警 LED** | PA11 | 超阈值闪烁报警 |
| **蜂鸣器** | PA12 | 超阈值鸣叫报警 |
| **USART1** | TX: PA9, RX: PA10 | 串口发送至上位机 |

### Keil 编译配置

- Options for Target → C/C++ → Include Paths 需添加：`User`、`Tasks`、`FreeRTOS/Source/include`、`FreeRTOS/Source/portable/RVDS/ARM_CM3`
- Target 需勾选 Use MicroLIB

---

## FreeRTOS 移植踩坑记录

### 故障现象06
编译报错 `identifier "sensorQueue" is undefined` 等符号未定义。

### 故障原因06
新建的 `Tasks/` 文件夹路径未添加到 Keil 的 C/C++ 包含路径中，编译器找不到 `task_sensor.h`。

### 解决方案06
Options for Target → C/C++ → Include Paths 添加 `Tasks` 文件夹路径。

---

### 故障现象07
串口持续打印 `DHT11 fail`，传感器在 FreeRTOS 下完全无法通信。

### 故障原因07
DHT11 通信时序为微秒级。FreeRTOS 的 SysTick 中断（每 1ms 一次）和任务调度会打断 DHT11 电平信号，导致通信失败。

### 解决方案07

多轮迭代：

- `taskENTER_CRITICAL()`：成功率从 0% 提升到约 20%，但仍有中断干扰
- `__disable_irq()`：彻底关闭所有中断，成功率 100%，但暂停 SysTick 心跳
- 最终方案：引入 **TIM2 硬件定时器延时**（`Delay_us_TIM` / `Delay_ms_TIM`），延时精度不依赖软件 NOP 循环，同时配合**任务优先级保护**（Sensor 优先级设为最高 3），其他任务无法抢占 Sensor 的 CPU 时间

---

### 故障现象08
传感器任务发送约 6 次数据（队列深度 5）后停止，串口不再有输出。

### 故障原因08
队列深度为 5，使用 `portMAX_DELAY` 等待发送。队列满后无消费者任务取走数据，传感器任务被永久阻塞在 `xQueueSend()`。

### 解决方案08
创建显示任务（`vTask_Display`）作为消费者，持续从队列取数据并刷新 OLED，释放队列空间。

---

### 故障现象09
编译报错 `L6200E: Symbol SysTick_Handler multiply defined`。

### 故障原因09
`stm32f10x_it.c` 和 FreeRTOS 的 `port.c` 同时定义了 `SysTick_Handler`，链接器发现重名符号。

### 解决方案09
- 启动文件向量表将 `SysTick_Handler` 改为 `xPortSysTickHandler`，指向 FreeRTOS 的 port.c 实现
- `stm32f10x_it.c` 中删除 `SysTick_Handler`，改为 `vApplicationTickHook()` 通过 FreeRTOS Tick Hook 递增 `g_millis`，保持与裸机版驱动代码的兼容

---

### 故障现象10
按一次按键 K1 后，串口短暂打印 `Key=1`，随后系统完全停止响应——串口不再打印温湿度数据，也不打印 `DHT11 fail`，整个系统陷入沉默。

### 故障原因10
Key 任务优先级（原为 2）与 Sensor 任务同级。按 K1 后 `Key_GetNum()` 内的 `while(等待松手)` 阻塞 Key 任务，同优先级的 Sensor 任务被时间片切换至 CPU 时，DHT11 的 GPIO 轮询循环（`while(GPIO_ReadInputDataBit(...) == 1)`）被 Key 任务抢占，错失电平变化窗口，导致 DHT11 死锁在等待信号变化的循环中。Sensor 任务卡死后不再生产数据，Display 和 Alarm 永久阻塞在 `xQueueReceive()`。

### 解决方案10
调整任务优先级：Sensor 升为 3（最高），Alarm 降为 2，Key 降为 1。Sensor 运行时任何任务都无法抢占，DHT11 通信时序得到保护。

---

## 待优化方向（未来计划）

- 按键状态机接入：keyQueue 消费者实现，将键值送入裸机版的状态机逻辑
- 按键扫描非阻塞化：将 `Key_GetNum()` 中的 while(等待松手) 改为外部中断 + 消抖定时器
- 阈值数据写入内部 Flash，实现掉电保存
- 利用 FreeRTOS 软件定时器替代 Alarm Task 中的 vTaskDelay 周期
- 利用 RTC 唤醒 + STOP 低功耗模式

---

## 相关链接

- [裸机版（标准库状态机）](../Temperature_Humidity_Sensor_Alarm_System_BareMetal/)
- [Python 串口上位机](https://github.com/Luoka666/upper_computer)
