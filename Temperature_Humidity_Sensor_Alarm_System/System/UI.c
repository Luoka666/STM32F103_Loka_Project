#include <stdio.h>
#include "stm32f10x.h"
#include "oled.h"
#include "dht11.h"
#include "key.h"

extern uint8_t temperature, humidity;
extern uint8_t temp_threshold, humi_threshold;
extern uint8_t menu_index;
extern uint8_t threshold_menu_index;
extern uint8_t keyNum;

uint8_t line_index = 1;
/* ========== 停滞界面 ========== */
void stop_ui(void) {

    OLED_ShowString(1, 1, "DHT11 Monitor");
    OLED_ShowString(2, 1, "State: STOP");
    OLED_ShowString(3, 1, "key1 to RUN");
    OLED_ShowString(4, 1, "key5 to setting");

}
/* ========== 运行界面 ========== */
void run_ui(uint8_t temp, uint8_t humi) {

    OLED_ShowString(1, 1, "DHT11 Monitor");
    OLED_ShowString(4, 1, "State: RUN  ");
    OLED_ShowString(2, 1, "Temp: ");
    OLED_ShowNum(2, 6, temp, 2);
    OLED_ShowChar(2, 8, 'C');
    OLED_ShowString(3, 1, "Humi: ");
    OLED_ShowNum(3, 6, humi, 2);
    OLED_ShowChar(3, 8, '%');

}
/* ========== 设置主菜单 ========== */

void setting_menu_ui(void) {
    char line2[17] = "  History        ";
    char line3[17] = "  Thresholds     ";
    char line4[17] = "  Back           ";//返回停止状态页面

    // 根据选中项绘制光标 '>'
    if (menu_index == 0)      line2[0] = '>';
    else if (menu_index == 1) line3[0] = '>';
    else if (menu_index == 2) line4[0] = '>';

    OLED_ShowString(1, 1, "Settings Menu  ");
    OLED_ShowString(2, 1, line2);
    OLED_ShowString(3, 1, line3);
    OLED_ShowString(4, 1, line4);
}
/* ========== 可更改选项展示页面 ========== */
void setting_change_ui(void) {
    char line2[17] = "  Temp           ";
    char line3[17] = "  Humi           ";
    char line4[17] = "  Back           ";

    if (threshold_menu_index == 0)      line2[0] = '>';
    else if (threshold_menu_index == 1) line3[0] = '>';
    else if (threshold_menu_index == 2) line4[0] = '>';

    OLED_ShowString(1, 1, "Set Thresholds  ");
    OLED_ShowString(2, 1, line2);
    OLED_ShowString(3, 1, line3);
    OLED_ShowString(4, 1, line4);
}
/* ========== 温度更改界面 ========== */
void setting_change_temp_ui(void) {
    OLED_ShowString(1, 1, "Set Temp Limit ");
    OLED_ShowString(2, 1, "Current: ");
    OLED_ShowNum(2, 9, temp_threshold, 2);
    OLED_ShowChar(2, 11, 'C');
    OLED_ShowString(3, 1, "UP/DN to change");
    OLED_ShowString(4, 1, "K5:Save K2:Back");
}
/* ========== 湿度更改界面 ========== */
void setting_change_humi_ui(void) {
    OLED_ShowString(1, 1, "Set Humi Limit ");
    OLED_ShowString(2, 1, "Current: ");
    OLED_ShowNum(2, 9, humi_threshold, 2);
    OLED_ShowChar(2, 11, '%');
    OLED_ShowString(3, 1, "UP/DN to change");
    OLED_ShowString(4, 1, "K5:Save K2:Back");
}
/* ========== 历史记录显示界面 ========== */
void setting_history_ui() {
    if (line_index == 5) {
        line_index = 1;
    }
    OLED_ShowString(line_index, 1, "T:");
    OLED_ShowNum(line_index, 3, temperature, 2);
    OLED_ShowChar(line_index, 5, 'C');
    OLED_ShowString(line_index, 7, "H:");
    OLED_ShowNum(line_index, 9, humidity, 2);
    OLED_ShowString(line_index, 11, "%");
    line_index ++;

}
