#include "stm32f10x.h"

void timer_Init(void) {

    //配置GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_IPU;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;

    //使能和配置外设模式
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_ETRClockMode2Config(TIM2,TIM_ExtTRGPSC_OFF,TIM_ExtTRGPolarity_Inverted,0x03);//通过ETR引脚的外部时钟模式2配置

    //配置时基单元
    TIM_TimeBaseInitTypeDef t;
    t.TIM_ClockDivision = TIM_CKD_DIV1;
    t.TIM_CounterMode = TIM_CounterMode_Up;
    t.TIM_Period = 10 - 1;
    t.TIM_Prescaler = 0;
    t.TIM_RepetitionCounter = 0;//高级定时器才使用的参数，不用直接给0
    TIM_TimeBaseInit(TIM2,&t);

    //清除TIM_TimeBaseStructInit();自动进入中断函数所造成的标志位
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);

    //打开TIM更新中断
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

    //配置NVIC
    NVIC_InitTypeDef s;
    s.NVIC_IRQChannel = TIM2_IRQn;
    s.NVIC_IRQChannelCmd = ENABLE;
    s.NVIC_IRQChannelPreemptionPriority = 2;
    s.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&s);
    //使能TIM2，定时器开始运行
    TIM_Cmd(TIM2, ENABLE);

}
