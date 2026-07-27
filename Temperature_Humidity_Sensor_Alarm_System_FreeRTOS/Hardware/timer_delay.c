#include "timer_delay.h"

// 初始化 TIM2 为 1MHz 计数频率（每个计数 = 1μs）
void TIM2_Delay_Init(void) {
    // 1. 开启 TIM2 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 2. 配置时基单元
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;        // 自动重装载值设为最大（65535μs ≈ 65ms）
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;     // 72MHz / 72 = 1MHz，每个计数 = 1μs
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;      // 不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
}

// 硬件定时器微秒延时（最大值约 65535μs）
void Delay_us_TIM(uint16_t xus) {
    TIM_SetCounter(TIM2, 0);           // 计数器清零
    TIM_Cmd(TIM2, ENABLE);             // 启动定时器
    while (TIM_GetCounter(TIM2) < xus); // 等待计数到达目标值
    TIM_Cmd(TIM2, DISABLE);            // 关闭定时器
}

// 硬件定时器毫秒延时（最大值约 65ms，因为计数器是 16 位、1MHz）
void Delay_ms_TIM(uint16_t xms) {
    uint16_t target = xms * 1000;       // 毫秒 → 微秒
    TIM_SetCounter(TIM2, 0);            // 计数器清零
    TIM_Cmd(TIM2, ENABLE);              // 启动定时器
    while (TIM_GetCounter(TIM2) < target); // 等待计数到达目标值
    TIM_Cmd(TIM2, DISABLE);             // 关闭定时器
}
