#include "stm32f10x.h"

void time_Init() {

    //打开时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

    //配置时钟源（选择为内部时钟）
    TIM_InternalClockConfig(TIM2);//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟

    //配置时基单元
    TIM_TimeBaseInitTypeDef p;
    p.TIM_ClockDivision = TIM_CKD_DIV1;
    p.TIM_CounterMode = TIM_CounterMode_Up;
    p.TIM_Prescaler = 7200 - 1;
    p.TIM_Period = 10000 - 1;
    p.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2,&p);
	
	/*中断输出配置*/
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);						//清除定时器更新标志位
																//TIM_TimeBaseInit函数末尾，手动产生了更新事件
																//若不清除此标志位，则开启中断后，会立刻进入一次中断

    //开启TIM更新中断
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

    //配置NVIC
    NVIC_InitTypeDef t;
    t.NVIC_IRQChannel = TIM2_IRQn;
    t.NVIC_IRQChannelCmd = ENABLE;
    t.NVIC_IRQChannelPreemptionPriority = 2;
    t.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&t);
	//使能TIM2，定时器开始运行
	TIM_Cmd(TIM2, ENABLE);

}
