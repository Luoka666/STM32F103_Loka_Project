# STM32F103 项目与外设练习

基于 **STM32F103C8T6 + C + 标准外设库** 的项目仓库。代表项目是温湿度监测与报警系统，包含裸机与 FreeRTOS 两套实现；其余目录保存外设入门练习和工程模板。

## 代表项目：温湿度监测与报警系统

实现 DHT11 温湿度采集、OLED 多级菜单、温湿度阈值设置、LED / 蜂鸣器报警、最近 4 条采样记录查看与串口数据上传。

| 版本 | 组织方式 | 项目入口 |
| --- | --- | --- |
| **裸机版** | 主循环 + 7 状态有限状态机；按键、采样和报警分别计时 | [功能、接线与调试记录](Temperature_Humidity_Sensor_Alarm_System_BareMetal/README.md) |
| **FreeRTOS 版** | Sensor / StateMachine / Alarm / Display / Record / Key 共 6 个任务；4 条消息队列与资源互斥 | [任务架构、队列与调试记录](Temperature_Humidity_Sensor_Alarm_System_FreeRTOS/README.md) |
| **配套上位机** | Python 串口接收与温湿度曲线显示，支持历史浏览和断线重连 | [upper_computer](https://github.com/Luoka666/upper_computer) |

### 重点实现

- **状态与界面**：按键事件决定状态跳转；状态切换清屏；RUN 状态下采样成功后更新温湿度显示。
- **独立节拍**：裸机按键扫描每 20ms 执行，FreeRTOS 按键任务每 10ms 扫描；RUN 连续运行期间每 2 秒尝试采样。
- **异常退出**：DHT11 电平等待设有超时，失败时释放总线并跳过本次数据更新。
- **队列防阻塞**：RTOS 显示、报警队列各保留一条最新数据，记录队列满时淘汰最旧待处理项。
- **共享资源**：互斥量分别保护 OLED 与历史记录访问，检查队列、互斥量及任务创建结果。
- **硬件联调**：记录 GPIO 配置、供电连接、SysTick 延时冲突和通信时序等问题的排查过程。

### 从哪里开始读

1. 先读对应版本的 README，了解接线、操作方法和整体架构。
2. 裸机版从 [User/main.c](Temperature_Humidity_Sensor_Alarm_System_BareMetal/User/main.c) 跟踪按键、采样与报警。
3. RTOS 版从 [User/main.c](Temperature_Humidity_Sensor_Alarm_System_FreeRTOS/User/main.c) 看任务创建，再看 [Tasks/task_sensor.c](Temperature_Humidity_Sensor_Alarm_System_FreeRTOS/Tasks/task_sensor.c) 的数据分发。
4. 根据需要阅读 `Hardware/`、`System/` 与 `Tasks/` 中的业务代码。

### 编译与运行

- 使用 Keil MDK 打开对应版本目录内的 `Project.uvprojx`。
- 参照子项目 README 完成接线和编译配置，烧录后默认进入 STOP。
- K1 切换运行 / 停止；STOP 下 K5 进入设置；K3 / K4 移动光标或调整阈值；K2 确认菜单；K5 返回。
- 连接上位机时使用 9600 bps，并将 Python 中的串口号改为设备实际 COM 口。

## 外设练习索引

以下为入门练习与模板。编号沿用原目录命名，独立于上面的两个综合项目。

| 目录 | 练习内容 |
| --- | --- |
| [1-3 Delay函数模块](1-3%20Delay函数模块/) | 延时函数与 SysTick |
| [2-1STM32projectexample](2-1STM32projectexample/) | STM32 工程模板与标准库配置 |
| [3-1-1LED flashing](3-1-1LED%20flashing/) | GPIO 输出与 LED 闪烁 |
| [3-1-1LED flashing_FreeRTOS](3-1-1LED%20flashing_FreeRTOS/) | LED 的 FreeRTOS 练习 |
| [3-1-2LED flowing light](3-1-2LED%20flowing%20light/) | LED 流水灯 |
| [3-1-2buzzer](3-1-2buzzer/) | 蜂鸣器输出控制 |
| [3-4-1Key-controlled LED](3-4-1Key-controlled%20LED/) | GPIO 输入与按键 |
| [3-4-2Lightsensor_controls_buzzer](3-4-2Lightsensor_controls_buzzer/) | 光敏模块控制蜂鸣器 |
| [4-1 OLED显示屏](4-1%20OLED显示屏/) | OLED 显示 |
| [5-2-1Line-of-sight_infrared_sensor](5-2-1Line-of-sight_infrared_sensor/) | 对射式红外传感器 |
| [5-2-2rotary_encoder](5-2-2rotary_encoder/) | 旋转编码器 |
| [6-1-1Timer_periodic_interrupt](6-1-1Timer_periodic_interrupt/) | 定时器周期中断 |
| [6-2-2External_clock_interrupt](6-2-2External_clock_interrupt/) | 外部时钟练习 |
| [6-3-1PWM_breathLight](6-3-1PWM_breathLight/) | PWM 呼吸灯 |
| [6-3-2servo_motor](6-3-2servo_motor/) | 舵机 PWM 控制 |
