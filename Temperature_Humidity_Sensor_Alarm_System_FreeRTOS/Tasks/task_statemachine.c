#include "task_statemachine.h"
#include "UI.h"
#include "oled.h"
#include "USART.h"
#include <stdio.h>
#include "task_key.h"

// 状态机任务：从按键队列接收键值，管理状态跳转和 UI 绘制
void vTask_StateMachine(void* pvParameters) {
    (void)pvParameters;
    uint8_t keyNum;
    static SystemState lastState = STOP;
	
    while (1) {
        // 等待按键队列中的数据
        if (xQueueReceive(keyQueue, &keyNum, portMAX_DELAY) == pdTRUE) { // 开机后如果没有任何按键事件，该任务就一直阻塞在队列上
            
            // 调试打印
            char buf[30];
            sprintf(buf, "State=%d, Key=%d\r\n", currentState, keyNum);
            USART_SendString(buf);

            /* ===== 第一层：按键到状态跳转 ===== */
            switch (currentState) {
                case STOP:
                    if (keyNum == KEY_RUN_STOP) currentState = RUN;
                    if (keyNum == KEY_SETTING_BACK) currentState = SETTING_MENU;
                    break;

                case RUN:
                    if (keyNum == KEY_RUN_STOP) currentState = STOP;
                    break;

                case SETTING_MENU:
                    if (keyNum == KEY_SETTING_BACK) currentState = STOP;
                    if (keyNum == KEY_CONFIRM) {
                        if (menu_index == 0) currentState = SETTING_HISTORY;
                        else if (menu_index == 1) currentState = SETTING_CHANGE;
                        else if (menu_index == 2) currentState = STOP;
                    }
                    if (keyNum == KEY_UP) menu_index = (menu_index > 0) ? menu_index - 1 : 2;
                    if (keyNum == KEY_DOWN) menu_index = (menu_index < 2) ? menu_index + 1 : 0;
                    break;

                case SETTING_HISTORY:
                    if (keyNum == KEY_SETTING_BACK) currentState = SETTING_MENU;
                    break;

                case SETTING_CHANGE:
                    if (keyNum == KEY_SETTING_BACK) currentState = SETTING_MENU;
                    if (keyNum == KEY_CONFIRM) {
                        if (threshold_menu_index == 0) currentState = SETTING_CHANGE_TEMP;
                        else if (threshold_menu_index == 1) currentState = SETTING_CHANGE_HUMI;
                    }
                    if (keyNum == KEY_UP) threshold_menu_index = 0;
                    if (keyNum == KEY_DOWN) threshold_menu_index = 1;
                    break;

                case SETTING_CHANGE_TEMP:
                    if (keyNum == KEY_SETTING_BACK) currentState = SETTING_CHANGE;
                    if (keyNum == KEY_UP) temp_threshold++;
                    if (keyNum == KEY_DOWN) temp_threshold--;
                    break;

                case SETTING_CHANGE_HUMI:
                    if (keyNum == KEY_SETTING_BACK) currentState = SETTING_CHANGE;
                    if (keyNum == KEY_UP) humi_threshold++;
                    if (keyNum == KEY_DOWN) humi_threshold--;
                    break;
            }

            /* 状态切换时清屏 */
            if (currentState != lastState) {
				// 先获取互斥锁
                xSemaphoreTake(oledMutex, portMAX_DELAY);
                OLED_Clear();
				// 交锁
                xSemaphoreGive(oledMutex);
                lastState = currentState;
            }

            /* ===== 第二层：状态到 UI 绘制 ===== */
			// 获取互斥锁
            xSemaphoreTake(oledMutex, portMAX_DELAY);
            switch (currentState) {
                case STOP:
                    stop_ui();
                    break;
                case SETTING_MENU:
                    setting_menu_ui();
                    break;
                case SETTING_HISTORY:
                    setting_history_ui(); // 历史记录绘画
                    break;
                case SETTING_CHANGE:
                    setting_change_ui();
                    break;
                case SETTING_CHANGE_TEMP:
                    setting_change_temp_ui();
                    break;
                case SETTING_CHANGE_HUMI:
                    setting_change_humi_ui();
                    break;
                case RUN:
                    // run_ui 由显示任务负责，这里不需要画
                    break;
            }
			
            xSemaphoreGive(oledMutex);
        }
    }
}
