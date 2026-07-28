#include "task_alarm.h"
#include "task_sensor.h"   // 使用 SensorData_t 和 alarmQueue 声明
#include "LED.h"
#include "Buzzer.h"
#include "USART.h"
extern uint8_t temp_threshold;
extern uint8_t humi_threshold;

// 报警任务：从独立报警队列接收数据，非阻塞式闪烁
void vTask_Alarm(void* pvParameters) {
    (void)pvParameters;
    SensorData_t data;

    while (1) {
        // 等待报警队列中的数据
        if (xQueueReceive(alarmQueue, &data, portMAX_DELAY) == pdTRUE) { // 检查是否成功接收
            // 判断是否超阈值
            if (data.temperature > temp_threshold || data.humidity > humi_threshold) {
                // 超阈值：LED和蜂鸣器以 500ms 周期闪烁
                LED1_ON();
                Buzzer_on();
                vTaskDelay(pdMS_TO_TICKS(500));

                LED1_OFF();
                Buzzer_off();
                vTaskDelay(pdMS_TO_TICKS(500));
            } else {
                // 未超阈值：确保关闭
                LED1_OFF();
                Buzzer_off();
                // 如果不需要实时更新，可以短暂延时避免频繁轮询
				// 不延时，让 xQueueReceive 自然阻塞，让生产者和消费者速度同步
//                vTaskDelay(pdMS_TO_TICKS(200));// 未超阈值：关灯，什么都不做，立刻回去等下一条数据，防止队列阻塞
            }
        }
    }
}
