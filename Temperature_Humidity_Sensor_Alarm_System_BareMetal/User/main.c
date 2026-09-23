#include "stm32f10x.h"
#include "dht11.h"
#include "oled.h"
#include "USART.h"
#include "key.h"
#include "UI.h"
#include "LED.h"
#include "alarm.h"
#include "Record_storage.h"

#define KEY_SCAN_PERIOD_MS      20U
#define DHT11_SAMPLE_PERIOD_MS  2000U

//变量定义
uint8_t temperature = 0, humidity = 0;
uint8_t keyNum = 0;
uint8_t menu_index = 0;
uint8_t threshold_menu_index = 0;
//初始报警阈值
uint8_t temp_threshold = 40, humi_threshold = 60; // 报警阈值



//系统状态
typedef enum {
    STOP = 0, //停滞状态
    RUN, //运行状态
    SETTING_MENU, //设置状态,只有在STOP状态，才能进入。
    SETTING_HISTORY, //显示历史记录状态
    SETTING_CHANGE, //显示温湿度可更改项状态
    SETTING_CHANGE_TEMP, //更改温度状态
    SETTING_CHANGE_HUMI, //更改湿度状态
} SystemState;

//定义按键
#define KEY_RUN_STOP  1
#define KEY_CONFIRM   2 //即为确认也为保存
#define KEY_UP        3
#define KEY_DOWN      4
#define KEY_SETTING_back   5 //k5在stop状态下为设置，在设置状态及其子状态下，为返回。
//定义默认状态
SystemState currentState = STOP; //系统默认停止

int main(void) {
    uint32_t last_key_time;
    uint32_t last_sensor_time;
    SystemState lastState;
    uint8_t sensor_valid = 0;

    currentState = STOP;

    // 初始化外设（外设不工作，一定要先看有没有初始化（哭）（哭）（哭）（哭））
    OLED_Init();
    DHT11_Init();
    usart_Init();
    Key_Init(); // 按键初始化（之前修好的那个）
    init_alarm(); 
    //配置SysTick。
    SysTick_Config(SystemCoreClock / 1000); // 1ms 中断
    OLED_Clear();

    stop_ui();
    last_key_time = millis();
    last_sensor_time = millis();
    lastState = currentState;

    while (1) {
        uint32_t now = millis();

        if ((uint32_t)(now - last_key_time) >= KEY_SCAN_PERIOD_MS) {
            last_key_time = now;
            keyNum = Key_GetNum();

            /* ===== 第一层：按键到状态跳转 ===== */
            switch (currentState) {
                case STOP:
                    if (keyNum == KEY_RUN_STOP) currentState = RUN; // K1 运行
                    if (keyNum == KEY_SETTING_back) currentState = SETTING_MENU; // K5 设置
                    break;

                case RUN:
                    if (keyNum == KEY_RUN_STOP) currentState = STOP; // K1 停止
                    // K2 在 RUN 状态下故意不处理，保证运行安全
                    break;

                case SETTING_MENU:
                    if (keyNum == 5) currentState = STOP; //k5可以返回
                    if (keyNum == KEY_CONFIRM) {
                        // K2 确认
                        if (menu_index == 0) currentState = SETTING_HISTORY;
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
                    if (keyNum == KEY_CONFIRM) {
                        if (threshold_menu_index == 0) currentState = SETTING_CHANGE_TEMP;
                        else if (threshold_menu_index == 1) currentState = SETTING_CHANGE_HUMI;
                        else currentState = SETTING_MENU;
                    }
                    if (keyNum == KEY_UP) threshold_menu_index = (threshold_menu_index > 0) ? threshold_menu_index - 1 : 2;
                    if (keyNum == KEY_DOWN) threshold_menu_index = (threshold_menu_index < 2) ? threshold_menu_index + 1 : 0;
                    break;

                case SETTING_CHANGE_TEMP:
                    if (keyNum == KEY_SETTING_back) currentState = SETTING_CHANGE; // K5 返回
                    if (keyNum == KEY_UP && temp_threshold < 99U) temp_threshold++;
                    if (keyNum == KEY_DOWN && temp_threshold > 0U) temp_threshold--;
                    break;

                case SETTING_CHANGE_HUMI:
                    if (keyNum == KEY_SETTING_back) currentState = SETTING_CHANGE; // K5 返回
                    if (keyNum == KEY_UP && humi_threshold < 99U) humi_threshold++;
                    if (keyNum == KEY_DOWN && humi_threshold > 0U) humi_threshold--;
                    break;
            }

            /* 状态切换时清屏 */
            if (currentState != lastState) {
                OLED_Clear();
                lastState = currentState;
                if (currentState == RUN) {
                    last_sensor_time = now - DHT11_SAMPLE_PERIOD_MS;
                } else {
                    sensor_valid = 0;
                }
            }

            if (keyNum != 0U) {
                switch (currentState) {
                    case STOP:                stop_ui(); break;
                    case RUN:                 run_ui(temperature, humidity); break;
                    case SETTING_MENU:        setting_menu_ui(); break;
                    case SETTING_HISTORY:     setting_history_ui(); break;
                    case SETTING_CHANGE:      setting_change_ui(); break;
                    case SETTING_CHANGE_TEMP: setting_change_temp_ui(); break;
                    case SETTING_CHANGE_HUMI: setting_change_humi_ui(); break;
                }
            }
        }

        if (currentState == RUN &&
            (uint32_t)(now - last_sensor_time) >= DHT11_SAMPLE_PERIOD_MS) {
            last_sensor_time = now;
            sensor_valid = data_Check(&temperature, &humidity);
            if (sensor_valid) {
                run_ui(temperature, humidity);
                usart_send(temperature, humidity);
                history_add(temperature, humidity);
            }
        }

        if (currentState == RUN && sensor_valid) {
            alarm_run(temperature, humidity);
        } else {
            alarm_run(0, 0);
        }
    }
}
