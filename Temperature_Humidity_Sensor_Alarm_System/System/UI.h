#include <stdio.h>
#include "stm32f10x.h"
#include "oled.h"
#include "dht11.h"

extern uint8_t temperature, humidity;      // 当前温湿度
extern uint8_t temp_threshold, humi_threshold; // 报警阈值
extern uint8_t menu_index;               // 设置主菜单选中项: 0-历史, 1-阈值, 2-返回
extern uint8_t threshold_menu_index;     // 阈值菜单选中项: 0-温度, 1-湿度

/* ========== 停滞界面 ========== */
void stop_ui() {

    OLED_ShowString(1, 1, "DHT11 Monitor");
    OLED_ShowString(2, 1, "State: STOP");
    OLED_ShowString(3, 1, "Waiting");
    OLED_ShowString(4, 1, "Press Key");

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
    char line4[17] = "  Back           ";

    // 根据选中项绘制光标 '>'
    if (menu_index == 0)      line2[0] = '>';
    else if (menu_index == 1) line3[0] = '>';
    else if (menu_index == 2) line4[0] = '>';

    OLED_ShowString(1, 1, "Settings Menu  ");
    OLED_ShowString(2, 1, line2);
    OLED_ShowString(3, 1, line3);
    OLED_ShowString(4, 1, line4);
}
