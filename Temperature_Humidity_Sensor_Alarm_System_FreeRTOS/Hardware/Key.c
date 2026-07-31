#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "USART.h"
// FreeRTOS软件定时器消抖所需库
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // 时钟使能 GPIOA 

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; // 或运算，可同时配置
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 改成 GPIOA 
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~5，返回0代表没有按键按下
  * 注意事项：此函数是阻塞式操作，当按键按住不放时，函数会卡住，直到按键松手
  */
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;		//定义变量，默认键码值为0
	
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0)			//读PA1输入寄存器的状态，如果为0，则代表按键1按下
	{
		Delay_ms(20);											//延时消抖
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0);	//等待按键松手
		Delay_ms(20);											//延时消抖
		KeyNum = 1;												//置键码为1
	}
	
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0);
		Delay_ms(20);
		KeyNum = 2;
	}

	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0);
		Delay_ms(20);
		KeyNum = 3;
	}

	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0);
		Delay_ms(20);
		KeyNum = 4;
	}

	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0);
		Delay_ms(20);
		KeyNum = 5;
	}

	
	return KeyNum;			//返回键码值，如果没有按键按下，所有if都不成立，则键码为默认值0
}


//==================== 软件定时器消抖 ====================//

extern QueueHandle_t keyQueue;
// 按键状态机变量
static uint8_t key_debounce[5] = {0, 0, 0, 0, 0};  // 消抖计数器
static uint8_t key_confirmed[5] = {0, 0, 0, 0, 0};  // 已确认标记

// 软件定时器回调函数（每 10ms 执行一次）
void vKeyTimerCallback(TimerHandle_t xTimer) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint8_t keyNum = 0;

    // 依次检查 K1~K5（PA1~PA5）
    uint16_t pins[5] = {GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5};

    for (int i = 0; i < 5; i++) {
        if (GPIO_ReadInputDataBit(GPIOA, pins[i]) == 0) {  // 按键按下（低电平）
            if (key_debounce[i] < 2) {
                key_debounce[i]++;  // 消抖计数加 1
            }
            if (key_debounce[i] >= 2 && key_confirmed[i] == 0) {
                // 连续 2 次检测到按下（20ms 消抖完成），确认按键
                key_confirmed[i] = 1;
                keyNum = i + 1;  // 键值 1~5
                xQueueSendFromISR(keyQueue, &keyNum, &xHigherPriorityTaskWoken);
            }
        } else {  // 按键松开
            key_debounce[i] = 0;    // 消抖计数清零
            key_confirmed[i] = 0;   // 确认标记清零
        }
    }

    // 如果有更高优先级的任务被唤醒，触发上下文切换
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
