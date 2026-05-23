#include "stm32f10x.h"
#include "Delay.h"

int main(void)
{
    // 1. 开启 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 2. 同时初始化 PA0 和 PA1 为推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;  // 两个引脚一起配
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    while (1)
    {
        // 3. 同时拉低 PA0 和 PA1
        GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
        Delay_ms(500);

        // 4. 同时拉高 PA0 和 PA1
        GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
        Delay_ms(500);
    }
}
