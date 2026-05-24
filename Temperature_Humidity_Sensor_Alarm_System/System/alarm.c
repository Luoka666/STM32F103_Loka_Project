#include "stm32f10x.h"
#include "LED.h"
extern uint8_t temp_threshold, humi_threshold;
extern uint8_t temperature, humidity;
//初始化蜂鸣器和led闪烁
void init_alarm(void) {
    LED_Init();

}
void alarm_run() {

    if (temperature > humi_threshold || humidity > humi_threshold) {
        LED1_flashing();
    }


}
