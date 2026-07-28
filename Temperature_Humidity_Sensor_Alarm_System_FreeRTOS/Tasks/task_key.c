#include "task_key.h"
#include "key.h"       // 你原来的按键驱动（Key_Init、Key_GetNum）
#include "USART.h"     // 调试打印
#include <stdio.h>


// 按键任务：每 20ms 扫描一次，检测到按键后通过队列发送键值
void vTask_Key(void* pvParameters) {
    (void)pvParameters;
    uint8_t keyNum;

    while (1) {
        // 调用原来的按键扫描函数（阻塞式，带消抖）
        keyNum = Key_GetNum();

        // 如果检测到有效按键（键值 1~5），发送到队列
        if (keyNum != 0) {
            // 调试：打印按键值
            char buf[20];
            sprintf(buf, "Key=%d\r\n", keyNum);
            USART_SendString(buf);

            // 发送到按键队列
            xQueueSend(keyQueue, &keyNum, portMAX_DELAY);
			
        }

        // 每 20ms 扫描一次
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
