#include "stm32f10x.h"
#include "LED.h"
#include <stdio.h>
#include "USART.h"
#include "delay.h"
//uint32_t last_blink_time = 0;  // 上次闪烁时间点
//uint8_t  alarm_state = 0;        // 当前报警模块状态
extern uint8_t temp_threshold, humi_threshold;
extern volatile uint32_t g_millis; 

// SysTick 中断服务函数。注释掉，避免重复定义，直接去stm32f10x_it.c里配置。
//void SysTick_Handler(void)
//{
//    g_millis++;//配置为每一毫秒递增
//}

// 获取毫秒
uint32_t millis(void)
{
    return g_millis;//获取当前毫秒计数
}
//初始化蜂鸣器和led闪烁
void init_alarm(void) {
	
    LED_Init();

}
//利用整体的while循环
void alarm_run(uint8_t temp, uint8_t humi) {
	
		
	// 加在函数里面，用 static 修饰（只初始化一次，记忆保留）
    static uint32_t last_blink_time = 0;
    static uint8_t  alarm_state = 0;
    // 判断是否超阈值
    if (temp > temp_threshold || humi > humi_threshold)
    {
        // 检查时间是否过了500ms，到达500ms
        if (millis() - last_blink_time >= 500)   // millis()，时间记录函数
        {
            last_blink_time = millis();   // 更新就时间节点为当前时间节点

            // 翻转 LED 状态
            if (alarm_state == 0) {
                LED1_ON();
                alarm_state = 1;
            } else {
                LED1_OFF();
                alarm_state = 0;
            }
        }
    }
    else  // 没超阈值，灭灯
    {
        LED1_OFF();
        alarm_state = 0;
    }
}

