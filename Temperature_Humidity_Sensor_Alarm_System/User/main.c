#include "stm32f10x.h"
#include "dht11.h"
#include "oled.h"
#include "delay.h"
#include "USART.h"
#include "key.h"
uint8_t temp = 0;
uint8_t humi = 0;
uint8_t keyNum = 0;

// 系统状态
typedef enum {
    STOP = 0,// 停滞状态
    RUN,// 运行状态
    SETTING_MENU,// 设置状态
    SETTING_HISTORY,// 显示历史记录状态
    SETTING_CHANGE,// 显示温湿度可更改项状态
    SETTING_CHANGE_TEMP,// 更改温度状态
    SETTING_CHANGE_HUMI,// 更改湿度状态
} SystemState;
// 定义按键
#define KEY_RUN_STOP  1
#define KEY_SETTING   2
#define KEY_UP        3
#define KEY_DOWN      4
#define KEY_CONFIRM   5

// 定义默认状态
SystemState CurrentState = STOP; // 系统默认停止

int main(void) {
    DHT11_Init();
    OLED_Init();
    usart_Init();
    Key_Init();

    OLED_Clear(); // 清屏


    while (1) {
        keyNum = Key_GetNum();

        // --- 第一部分：按键控制状态跳转 ---
        if (keyNum != 0) { // 只有按键按下才处理
            SystemState nextState = CurrentState; // 预设下一个状态为当前状态

            // 全局逻辑：K1 永远负责启停
            if (keyNum == KEY_RUN_STOP) {
                nextState = (CurrentState == RUN) ? STOP : RUN;
            }
            else {
                // 局部逻辑：根据当前状态决定按键功能
                switch (CurrentState) {
                    case STOP:
                        if (keyNum == KEY_SETTING) nextState = SETTING_MENU;
                        break;

                    case SETTING_MENU:
                        if (keyNum == KEY_CONFIRM) nextState = SETTING_CHANGE;
                        if (keyNum == KEY_SETTING) nextState = STOP; // 返回
                        break;

                    case SETTING_CHANGE:
                        if (keyNum == KEY_CONFIRM) nextState = SETTING_MENU; // 确认保存并返回
                        break;

                        // K3, K4 在这里不改变状态，而是在“状态控制行为”里改变具体的变量值
                    default: break;
                }
            }

            // 判定状态是否真的改变了
            if (nextState != CurrentState) {
                CurrentState = nextState;
                OLED_Clear(); // 只有状态切换时才清屏
            }
        }

        // --- 第二部分：状态控制行为 ---
        switch (CurrentState) {
            case RUN:
                // 1. 读取DHT11
                // 2. 检查报警阈值
                // 3. 刷新数值显示
                break;
            case SETTING_MENU:
                // 显示菜单文字，并根据 keyNum (K3/K4) 移动光标
                break;
            case STOP:
                // 显示待机界面
                break;
            default: break;
        }
    }

}
