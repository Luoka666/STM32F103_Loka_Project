#include "task_sensor.h"
#include "dht11.h"      // 原来的 DHT11 驱动
#include "USART.h"
#include "LED.h"
#include "task_statemachine.h" // 获取当前系统状态

// 传感器采集任务：每 100ms 采集一次，通过队列发送给其他任务

void vTask_Sensor(void *pvParameters) {
    (void) pvParameters; // 空变量，防止编译器报警
    uint8_t temperature = 0, humidity = 0; // 存检测数据的中间变量
    SensorData_t data;

    while (1) {
        // RUN状态下工作
        if (currentState == RUN) {
            // 采集数据（直接调用原来写好的底层驱动函数）
            if (data_Check(&temperature, &humidity)) {
                // 采集成功，直接采集，不需要关中断了
                data.temperature = temperature; //拿取数据
                data.humidity = humidity;
                usart_send(data.temperature, data.humidity); //验证是否采集成功

                // 把数据塞进队列，发给显示任务和报警任务，每个任务消费者必须单独设置各自的队列，否则就会抢数据
                //			USART_SendString("Before xQueue\r\n");  // ← 加这行
                xQueueSend(sensorQueue, &data, portMAX_DELAY); // portMAX_DELAY 一直等，直到塞成功为止
                //			USART_SendString("After xQueue\r\n");   // ← 加这行
                xQueueSend(alarmQueue, &data, portMAX_DELAY); // 发给报警任务
                xQueueSend(recordQueue, &data, 0); // 发给历史存储任务
            } else {
                USART_SendString("DHT11 fail\r\n");
            }
        }

        // 100ms 后再次采集
        vTaskDelay(pdMS_TO_TICKS(100)); // 100/1000/1000 == 100ms，1tick == 1ms
    }
}
