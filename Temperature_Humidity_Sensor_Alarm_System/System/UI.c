#include <stdio.h>
#include "stm32f10x.h"
#include "oled.h"
#include "Record_storage.h"
#include "dht11.h"
#include "key.h"

extern uint8_t temperature, humidity;
extern uint8_t temp_threshold, humi_threshold;
extern uint8_t menu_index;
extern uint8_t threshold_menu_index;
extern uint8_t keyNum;
extern uint8_t history_temp[];
extern uint8_t history_humi[];
extern uint8_t history_count;
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
    OLED_ShowString(4, 1, "K5:Back and Save");
}
/* ========== 湿度更改界面 ========== */
void setting_change_humi_ui(void) {
    OLED_ShowString(1, 1, "Set Humi Limit ");
    OLED_ShowString(2, 1, "Current: ");
    OLED_ShowNum(2, 9, humi_threshold, 2);
    OLED_ShowChar(2, 11, '%');
    OLED_ShowString(3, 1, "UP/DN to change");
    OLED_ShowString(4, 1, "K5:Back and Save");
}
/* ========== 历史记录显示界面 ========== */
//显示的是实时值，并没有成功存储为历史记录，意思是我在关闭系统后，这些记录会瞬间消失，并不会存储下来
//使用了移位寄存器方式（history_add(temperature, humidity)），并显示出来
void setting_history_ui() {
    uint8_t i;

    // 如果还没有存储任何记录，显示空白
    if (history_count == 0)
    {
        for (i = 0; i < HISTORY_SIZE; i++)
        {
            OLED_ShowString(i + 1, 1, "                "); // 清空4行
        }
        return;
    }

    // 从第1行开始，逐行显示历史数据（最新数据在最下面）
    for (i = 0; i < history_count; i++)
    {
        OLED_ShowString(i + 1, 1, "T:");
        OLED_ShowNum(i + 1, 3, history_temp[i], 2);
        OLED_ShowChar(i + 1, 5, 'C');
        OLED_ShowString(i + 1, 7, "H:");
        OLED_ShowNum(i + 1, 9, history_humi[i], 2);
        OLED_ShowChar(i + 1, 11, '%');
    }

    // 如果还没有存满4条，用空行填充剩余行，避免残留旧字符
    for (i = history_count; i < HISTORY_SIZE; i++)
    {
        OLED_ShowString(i + 1, 1, "                "); // 清空该行
    }
}


