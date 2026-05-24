#include "stm32f10x.h"
#include "dht11.h"
#include "oled.h"
#include "delay.h"
#include "USART.h"
#include "key.h"
#include "UI.h"
#include "LED.h"
#include "alarm.h"
#include <stdio.h>
uint8_t temperature = 0, humidity = 0;
uint8_t keyNum = 0;
uint8_t menu_index = 0;
uint8_t threshold_menu_index = 0;
//初始报警阈值
uint8_t temp_threshold = 70, humi_threshold = 60; // 报警阈值

//系统状态
typedef enum {
    STOP = 0,//停滞状态
    RUN,//运行状态
    SETTING_MENU,//设置状态,只有在STOP状态，才能进入。
    SETTING_HISTORY,//显示历史记录状态
    SETTING_CHANGE,//显示温湿度可更改项状态
    SETTING_CHANGE_TEMP,//更改温度状态
    SETTING_CHANGE_HUMI,//更改湿度状态
} SystemState;
//定义按键
#define KEY_RUN_STOP  1
#define KEY_CONFIRM   2 //即为确认也为保存
#define KEY_UP        3
#define KEY_DOWN      4
#define KEY_SETTING_back   5 //k5在stop状态下为设置，在设置状态及其子状态下，为返回。
//定义默认状态
SystemState currentState = STOP; //系统默认停止

int main(void){


    currentState = STOP;

    // 初始化外设...
    OLED_Init();
    DHT11_Init();
    usart_Init();
	Key_Init();    // 按键初始化（之前修好的那个）
	LED_Init();    // LED 初始化

    OLED_Clear();

	while (1){

        keyNum = Key_GetNum();  // 非阻塞，无按键返回 0
		
		//串口显示按键参数，用于调试
//		if (keyNum != 0) {
//		char buf[40];
//		sprintf(buf, "keyNum=%d, state=%d\r\n", keyNum, currentState);
//		USART_SendString(buf);
//		}

		static SystemState lastState = STOP; // 记录上一次的状态

        /* ===== 第一层：按键 → 状态跳转 ===== */
        switch (currentState)
        {
            case STOP:
                if (keyNum == KEY_RUN_STOP) currentState = RUN;        // K1 运行
                if (keyNum == KEY_SETTING_back) currentState = SETTING_MENU; // K5 设置
                break;

            case RUN:
                if (keyNum == KEY_RUN_STOP) currentState = STOP;        // K1 停止
                // K2 在 RUN 状态下故意不处理，保证运行安全
                break;

            case SETTING_MENU:
                if (keyNum == KEY_CONFIRM) {                           // K2 确认
                    if (menu_index == 0)      currentState = SETTING_HISTORY;
                    else if (menu_index == 1) currentState = SETTING_CHANGE;
                    else if (menu_index == 2) currentState = STOP;
                }
                // K3/K4 在 MENU 状态下只改变 menu_index，不跳转状态
                if (keyNum == 3) menu_index = (menu_index > 0) ? menu_index - 1 : 2;
                if (keyNum == 4) menu_index = (menu_index < 2) ? menu_index + 1 : 0;
                break;

            case SETTING_HISTORY:
                if (keyNum == 5) currentState = SETTING_MENU; // K5 返回
                break;

            case SETTING_CHANGE:
                if (keyNum == KEY_SETTING_back) currentState = SETTING_MENU; // K5 返回
                if (keyNum == KEY_CONFIRM) {                            // K2 确认
                    if (threshold_menu_index == 0)      currentState = SETTING_CHANGE_TEMP;
                    else if (threshold_menu_index == 1) currentState = SETTING_CHANGE_HUMI;
                }
                if (keyNum == 3) threshold_menu_index = 0;
                if (keyNum == 4) threshold_menu_index = 1;
                break;

            case SETTING_CHANGE_TEMP:
                if (keyNum == KEY_SETTING_back) currentState = SETTING_CHANGE; // K5 返回
//                if (keyNum == 2) { /* 保存阈值 */ currentState = SETTING_CHANGE; }// k2保存
                // K3/K4 调整阈值数值
                if (keyNum == 3) temp_threshold++;
                if (keyNum == 4) temp_threshold--;
                break;

            case SETTING_CHANGE_HUMI:
                if (keyNum == KEY_SETTING_back) currentState = SETTING_CHANGE; // K5 返回
//                if (keyNum == 2) { /* 保存阈值 */ currentState = SETTING_CHANGE; }
                if (keyNum == 3) humi_threshold++;
                if (keyNum == 4) humi_threshold--;
                break;

        }

		/* 状态切换时清屏 */
        if (currentState != lastState) {
            OLED_Clear();
            lastState = currentState;
        }


        /* ===== 第二层：状态 → 行为执行 ===== */
        switch (currentState)
        {
            case STOP:
                stop_ui();
                break;

            case RUN:
                data_Check(&temperature, &humidity);          // 采集数据
                run_ui(temperature, humidity);                // 更新显示// 报警判断
                usart_send(temperature, humidity);             // 串口发送
                break;

            case SETTING_MENU:
                setting_menu_ui();
                break;

            case SETTING_HISTORY:
                setting_history_ui();
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

        }

        Delay_ms(100);
    }

}
