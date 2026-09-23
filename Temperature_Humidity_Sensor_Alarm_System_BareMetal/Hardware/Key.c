#include "stm32f10x.h"                  // Device header

#define KEY_DEBOUNCE_SAMPLES  2U

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
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;// 或运算可
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
	static uint8_t last_sample = 0;
	static uint8_t stable_key = 0;
	static uint8_t stable_count = 0;
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

	if (stable_count < KEY_DEBOUNCE_SAMPLES) {
		stable_count++;
	}

	if (stable_count >= KEY_DEBOUNCE_SAMPLES && sample != stable_key) {
		stable_key = sample;
		if (stable_key != 0) {
			return stable_key;
		}
	}

	return 0;
}
