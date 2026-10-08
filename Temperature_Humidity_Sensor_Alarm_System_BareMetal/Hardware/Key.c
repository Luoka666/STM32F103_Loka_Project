#include "stm32f10x.h"                  // Device header

#define KEY_DEBOUNCE_SAMPLES  2U // 设置稳定期望次数，两次按键值同样认为稳定

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
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;// 或运算可同时使能1--5
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 改成 GPIOA
}

/**
  * 函    数：按键获取键码
  * 参    数：无
  * 返 回 值：按下按键的键码值，范围：0~5，返回0代表没有按键按下
  * 注意事项：非阻塞消抖；需要由主循环每 20ms 左右调用一次
  */
uint8_t Key_GetNum(void)
{
	static uint8_t last_sample = 0; // 记录上一次采样到的按键值
	static uint8_t stable_key = 0; // 已经稳定且成功采样的按键，可用于识别该按键是否被长按。
	static uint8_t stable_count = 0; // 按键连续次数记录
	uint8_t sample = 0;

	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0) sample = 1;
	else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0) sample = 2;
	else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0) sample = 3;
	else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0) sample = 4;
	else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0) sample = 5;

	if (sample != last_sample) {
		last_sample = sample;
		stable_count = 1;
		return 0;
	}

	if (stable_count < KEY_DEBOUNCE_SAMPLES) { // 只有连续二次采样相同才会执行该if语句
		stable_count++;
	}

	if (stable_count >= KEY_DEBOUNCE_SAMPLES && sample != stable_key) { // 确认稳定按键与防止长按
		stable_key = sample;
		if (stable_key != 0) { // 按键松开消抖最终判断
			return stable_key;
		}
	}

	return 0;
}
