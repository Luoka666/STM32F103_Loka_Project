#include "task_display.h"
#include "task_sensor.h"   // SensorData_t 和 sensorQueue 在这里声明
#include "oled.h"           //  原来的 OLED 驱动
#include <stdio.h>
#include "UI.h"
#include "task_statemachine.h" // 互斥锁声明

void vTask_Display(void* pvParameters) {
    (void)pvParameters;
    SensorData_t data;
    

    while (1) {
        // 从队列取数据，队列为空则阻塞等待
        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdTRUE) { // 有数据就立刻刷新
			xSemaphoreTake(oledMutex, portMAX_DELAY); // 获取互斥锁
            // 刷新 OLED 显示
			if (currentState == RUN) // 判断当前全局状态变量状态，避免run_ui()会和stop_ui()重合
			{           
				run_ui(data.temperature, data.humidity);
			}                                    
			// 交锁退出
			xSemaphoreGive(oledMutex);
        }
		
        // 不需要 vTaskDelay，xQueueReceive 本身就是阻塞点
//		队列就是该任务的节奏控制器。 
//		传感器每 100ms 发一帧数据，显示任务就每 100ms 刷一次屏幕，节奏完全由数据流决定，不需要额外延时。
		
    }
}
