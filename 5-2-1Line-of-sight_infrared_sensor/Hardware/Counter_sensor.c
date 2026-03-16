#include "stm32f10x.h"
#include <stdint.h>
#include "Delay.h"
//计数器
uint32_t counter  = 0;

//各部初始化函数
void sensor_Init(void) {

    //配置GPIO和AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_IPU;
    p.GPIO_Pin = GPIO_Pin_0;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&p);

    //指定AFIO的中断引脚选择
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);

    //配置EXTI
    EXTI_InitTypeDef s;
    s.EXTI_Line = EXTI_Line0;
    s.EXTI_LineCmd = ENABLE;
    s.EXTI_Mode = EXTI_Mode_Interrupt;
    s.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&s);

    //设置优先级分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    //配置NVIC
    NVIC_InitTypeDef t;
    t.NVIC_IRQChannel = EXTI0_IRQn ;
    t.NVIC_IRQChannelCmd = ENABLE;
    t.NVIC_IRQChannelPreemptionPriority = 1;
    t.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&t);

}
//获取计数
uint32_t get_Counter(void) {

    return counter;

}
//中断执行函数(每一种中断线都有固定的中断函数)
void EXTI0_IRQHandler(void) {

    Delay_ms(20);

    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {

        counter++;

    }

    //清除标志位
    EXTI_ClearITPendingBit(EXTI_Line0);

}