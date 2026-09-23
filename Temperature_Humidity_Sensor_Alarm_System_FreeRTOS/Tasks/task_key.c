#include "task_key.h"
#include "key.h"       // 你原来的按键驱动（Key_Init、Key_GetNum）
// 按键任务：每 10ms 扫描一次，驱动层完成非阻塞消抖
void vTask_Key(void* pvParameters) {
    (void)pvParameters;
    uint8_t keyNum;

    while (1) {
        // 非阻塞扫描；每次稳定按下只产生一个事件
        keyNum = Key_GetNum();

        // 如果检测到有效按键（键值 1~5），发送到队列
        if (keyNum != 0) {
            // 按键任务不能因队列满而永久阻塞；人机输入允许丢弃过期事件
            (void)xQueueSend(keyQueue, &keyNum, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
