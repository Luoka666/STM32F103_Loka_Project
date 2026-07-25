#include "task_display.h"
#include "task_sensor.h"   // SensorData_t 和 sensorQueue 在这里声明
#include "oled.h"           //  原来的 OLED 驱动
#include <stdio.h>
#include "UI.h"
void vTask_Display(void* pvParameters) {
    (void)pvParameters;
    SensorData_t data;
    

    while (1) {
        // 从队列取数据，队列为空则阻塞等待
        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdTRUE) {
            // 刷新 OLED 显示
			run_ui(data.temperature, data.humidity);
        }
        // 不需要 vTaskDelay，xQueueReceive 本身就是阻塞点
    }
}
