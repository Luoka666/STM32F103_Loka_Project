#include "task_sensor.h"
#include "dht11.h"      // 原来的 DHT11 驱动

// 传感器采集任务：每 100ms 采集一次，通过队列发送给其他任务

void vTask_Sensor(void* pvParameters) {
	
    (void)pvParameters;// 空变量，防止编译器报警
    uint8_t temperature = 0, humidity = 0;// 存检测数据的中间变量
    SensorData_t data;

    while (1) {
		
        // 采集数据（直接调用原来写好的底层驱动函数）
        if (data_Check(&temperature, &humidity)) {  // 采集成功
            data.temperature = temperature;
            data.humidity = humidity;
            // 把数据塞进队列，发给显示任务和报警任务
            xQueueSend(sensorQueue, &data, portMAX_DELAY);
        }
		
        // 100ms 后再次采集
        vTaskDelay(pdMS_TO_TICKS(100));// 100/1000/1000 == 100ms，1tick == 1ms
		
    }
	
}
