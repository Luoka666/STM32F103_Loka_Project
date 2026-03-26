#include "stm32f10x.h"

void breathLight(void) {
    //打开定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    //打开GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

    //配置GPIO口
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    //调用内部时钟
    TIM_InternalClockConfig(TIM2);

    //配置时基单元
    TIM_TimeBaseInitTypeDef p;
    p.TIM_ClockDivision = TIM_CKD_DIV1;
    p.TIM_CounterMode = TIM_CounterMode_Up;
    p.TIM_Period = 1000 - 1;    //ARR，//频率设置为100hz
    p.TIM_Prescaler = 720 - 1;   //PSC
    p.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &p);


    //配置输出比较单元
    TIM_OCInitTypeDef oc;
    //配置结构体参数默认值
    TIM_OCStructInit(&oc);

    oc.TIM_OCMode = TIM_OCMode_PWM1;    //选择TWM模式1
    oc.TIM_OCPolarity = TIM_OCPolarity_High;    //控制REF，不反转波形，正常输出。
    oc.TIM_OutputState = TIM_OutputState_Enable ;
    oc.TIM_Pulse = 0;    //CCR的值，初始为0，逐渐递增。
    TIM_OC1Init(TIM2,&oc);

    //使能TIM2，定时器开始运行
    TIM_Cmd(TIM2, ENABLE);

}
