#include "task_record.h"
#include "task_sensor.h"      // SensorData_t 和 recordQueue 声明
#include "Record_storage.h"   // 裸机版的历史记录存储函数
#include "USART.h"
#include "task_statemachine.h"
// 历史记录存储任务：从独立队列接收数据，写入环形缓冲区
void vTask_Record(void* pvParameters) {
    (void)pvParameters;
    SensorData_t data;

    while (1) {
        // 等待记录队列中的数据
        if (xQueueReceive(recordQueue, &data, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(historyMutex, portMAX_DELAY) == pdTRUE) {
                history_add(data.temperature, data.humidity);
                xSemaphoreGive(historyMutex);
            }

        }
        // 不需要 vTaskDelay，xQueueReceive 本身就是阻塞点，和显示任务一样的道理。
    }
	
}
