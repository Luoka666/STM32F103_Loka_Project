#include "task_statemachine.h"
#include "UI.h"
#include "oled.h"
#include "USART.h"
#include <stdio.h>
#include "task_key.h"
#include "Threshold_storage.h"

/* 目前只在非 RUN 的阈值编辑页保存，不把 Flash 擦写塞进采集任务。
 * 保存操作由本任务独占；Flash 等待 BUSY 时会让出 CPU。
 * OLED 锁只用于画提示，绝不拿着 OLED 锁等待整个擦写过程。
 */
static uint8_t SaveThresholds(void) {
    ThresholdStore_Result result;
    char message[80];
    xSemaphoreTake(oledMutex, portMAX_DELAY);
    OLED_ShowString(4, 1, "Saving...       ");
    xSemaphoreGive(oledMutex);
    result = ThresholdStore_Save(temp_threshold, humi_threshold);
    sprintf(message, "Threshold save=%s driver=%u T=%u H=%u\r\n",
            ThresholdStore_ResultText(result), (unsigned int)ThresholdStore_GetFlashResult(),
            temp_threshold, humi_threshold);
    USART_SendString(message);
    return result == THRESHOLD_STORE_OK || result == THRESHOLD_STORE_UNCHANGED;
}

// 状态机任务：从按键队列接收键值，管理状态跳转和 UI 绘制
void vTask_StateMachine(void* pvParameters) {
    (void)pvParameters;
    uint8_t keyNum;
    uint8_t save_failed = 0; // 保存失败时留在当前编辑页，K5 可重试。
    static SystemState lastState = STOP;
	
    while (1) {
        // 等待按键队列中的数据
        if (xQueueReceive(keyQueue, &keyNum, portMAX_DELAY) == pdTRUE) { // 开机后如果没有任何按键事件，该任务就一直阻塞在队列上
            
            // 调试打印
//            char buf[30];
//            sprintf(buf, "State=%d, Key=%d\r\n", currentState, keyNum);
//            USART_SendString(buf);

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
                        save_failed = 0;
                        if (threshold_menu_index == 0) currentState = SETTING_CHANGE_TEMP;
                        else if (threshold_menu_index == 1) currentState = SETTING_CHANGE_HUMI;
                        else currentState = SETTING_MENU;
                    }
                    if (keyNum == KEY_UP) threshold_menu_index = (threshold_menu_index > 0) ? threshold_menu_index - 1 : 2;
                    if (keyNum == KEY_DOWN) threshold_menu_index = (threshold_menu_index < 2) ? threshold_menu_index + 1 : 0;
                    break;

                case SETTING_CHANGE_TEMP:
                    /* 上下键只修改 RAM；K5 才同时保存两个阈值。
                     * 只有真正写入并读回验证成功，或与已保存值相同，才返回。
                     */
                    if (keyNum == KEY_SETTING_BACK) {
                        save_failed = (uint8_t)!SaveThresholds();
                        if (!save_failed) currentState = SETTING_CHANGE;
                    }
                    if (keyNum == KEY_UP && temp_threshold < 99) temp_threshold++;
                    if (keyNum == KEY_DOWN && temp_threshold > 0) temp_threshold--;
                    break;

                case SETTING_CHANGE_HUMI:
                    if (keyNum == KEY_SETTING_BACK) {
                        save_failed = (uint8_t)!SaveThresholds();
                        if (!save_failed) currentState = SETTING_CHANGE;
                    }
                    if (keyNum == KEY_UP && humi_threshold < 99) humi_threshold++;
                    if (keyNum == KEY_DOWN && humi_threshold > 0) humi_threshold--;
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
                    if (xSemaphoreTake(historyMutex, portMAX_DELAY) == pdTRUE) {
                        setting_history_ui();
                        xSemaphoreGive(historyMutex);
                    }
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
			
            if (save_failed && (currentState == SETTING_CHANGE_TEMP ||
                                currentState == SETTING_CHANGE_HUMI)) {
                /* 16 字符覆盖整行，避免旧的 Back and Save 字符残留。
                 * 失败时修改值仍在 RAM，但并未保证掉电保存。
                 */
                OLED_ShowString(4, 1, "FAIL K5 to retry ");
            }
            xSemaphoreGive(oledMutex);
        }
    }
}
