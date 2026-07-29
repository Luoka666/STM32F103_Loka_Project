#include "task_record.h"
#include "task_sensor.h"      // SensorData_t 和 recordQueue 声明
#include "Record_storage.h"   // 裸机版的历史记录存储函数
#include "USART.h"
// 历史记录存储任务：从独立队列接收数据，写入环形缓冲区
void vTask_Record(void* pvParameters) {
    (void)pvParameters;
    SensorData_t data;

    while (1) {
        // 等待记录队列中的数据
        if (xQueueReceive(recordQueue, &data, portMAX_DELAY) == pdTRUE) {
            // 直接复用裸机版的存储函数（环形缓冲区）
            history_add(data.temperature, data.humidity);

        }
        // 不需要 vTaskDelay，xQueueReceive 本身就是阻塞点，和显示任务一样的道理。
    }
	
}
