#include "task_sensor.h"
#include "dht11.h"      // 原来的 DHT11 驱动
#include "USART.h"
#include "LED.h"

// 传感器采集任务：每 100ms 采集一次，通过队列发送给其他任务

void vTask_Sensor(void* pvParameters) {
	
    (void)pvParameters;// 空变量，防止编译器报警
    uint8_t temperature = 0, humidity = 0;// 存检测数据的中间变量
    SensorData_t data;

    while (1) {
		
		// 挂起调度器：禁止任务切换，但不关闭SysTick等中断
        vTaskSuspendAll();
        uint8_t result = data_Check(&temperature, &humidity);
        xTaskResumeAll();  // 恢复调度器
		
//		// 彻底关闭所有中断（包括 SysTick）
//        __disable_irq();
//        uint8_t result = data_Check(&temperature, &humidity);
//        __enable_irq();  // 恢复中断
 
		
        // 采集数据（直接调用原来写好的底层驱动函数）
        if (result) {  // 采集成功
            data.temperature = temperature;//拿取数据
            data.humidity = humidity;
			usart_send(data.temperature, data.humidity);//验证是否采集成功
            // 把数据塞进队列，发给显示任务和报警任务
            xQueueSend(sensorQueue, &data, 0);// portMAX_DELAY 一直等，直到塞成功为止
        }else{
			USART_SendString("DHT11 fail\r\n");
		}
		
        // 100ms 后再次采集
        vTaskDelay(pdMS_TO_TICKS(100));// 100/1000/1000 == 100ms，1tick == 1ms
		
    }
	
}
