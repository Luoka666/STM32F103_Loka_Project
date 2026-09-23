#include "task_sensor.h"
#include "dht11.h"      // 原来的 DHT11 驱动
#include "USART.h"
#include "LED.h"
#include "task_statemachine.h" // 获取当前系统状态

// DHT11 的刷新周期较慢，连续读取至少间隔 2 秒
#define DHT11_SAMPLE_PERIOD_MS  2000U

void vTask_Sensor(void *pvParameters) {
    (void) pvParameters; // 空变量，防止编译器报警
    uint8_t temperature = 0, humidity = 0; // 存检测数据的中间变量
    SensorData_t data;
    TickType_t last_sample_tick = 0;
    uint8_t was_running = 0;

    while (1) {
        TickType_t now = xTaskGetTickCount();

        // RUN状态下工作
        if (currentState == RUN) {
            if (!was_running || (now - last_sample_tick) >= pdMS_TO_TICKS(DHT11_SAMPLE_PERIOD_MS)) {
                was_running = 1;
                last_sample_tick = now;

                if (data_Check(&temperature, &humidity)) {
                    data.temperature = temperature;
                    data.humidity = humidity;
                    usart_send(data.temperature, data.humidity);

                    // 显示和报警只关心最新值，长度为 1 的队列使用覆盖写避免反压
                    (void)xQueueOverwrite(sensorQueue, &data);
                    (void)xQueueOverwrite(alarmQueue, &data);

                    // 历史队列满时丢掉最旧待处理项，再写入最新数据
                    if (xQueueSend(recordQueue, &data, 0) != pdPASS) {
                        SensorData_t discarded;
                        (void)xQueueReceive(recordQueue, &discarded, 0);
                        (void)xQueueSend(recordQueue, &data, 0);
                    }
                } else {
                    USART_SendString("DHT11 timeout/checksum fail\r\n");
                }
            }
        } else {
            was_running = 0;
        }

        // 高频检查状态，真正采样仍严格受 2 秒周期限制
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
