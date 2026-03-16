#include "stm32f10x.h"


//由于 counter 的值会随时被中断函数改变，编译器在优化时可能会把 main 中读取到的值缓存在寄存器里。
//导致即使中断改变了 counter，main 函数里的数值显示也不更新。
//声明变量时加上 volatile。这将强制编译器每次读取 counter 时都去内存里拿最新值。

volatile int32_t counter = 0;

void encoder_Init(void) {

    //打开GPIO和AFIO的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

    //配置GPIO
    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_IPU;
    p.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&p);

    //指定AFIO选择引脚（非位掩码不能使用或运算）
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);

    //配置EXTI
    EXTI_InitTypeDef s;
    s.EXTI_Line = EXTI_Line0 | EXTI_Line1;
    s.EXTI_LineCmd = ENABLE;
    s.EXTI_Mode = EXTI_Mode_Interrupt;
    s.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&s);

    //配置NVIC
    NVIC_InitTypeDef t;

    // 公共配置
    t.NVIC_IRQChannelCmd = ENABLE;
    t.NVIC_IRQChannelPreemptionPriority = 1;
    t.NVIC_IRQChannelSubPriority = 1;

    // 初始化 EXTI0
    t.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_Init(&t);

    // 初始化 EXTI1
    t.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_Init(&t);

}

//中断函数EXTI0_IRQn
void EXTI0_IRQHandler(void) {



    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        //A相跳变（但还不知道A相是在B相之前还是之后跳变）
        if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0) {//判断B相状态，如果B相为0，则说明A相后跳变，逆时针，counter--

            counter--;

        }else {//判断B相状态，如果B相为1，则说明A相先跳变，顺时针，counter++

            counter++;

        }

    }

    EXTI_ClearITPendingBit(EXTI_Line0);

}

//中断函数EXTI1_IRQn
void EXTI1_IRQHandler(void) {

    //B相跳变（但还不知道B相是在A相之前还是之后跳变）
    if (EXTI_GetITStatus(EXTI_Line1) != RESET) {

        if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) == 0) {//判断A相状态，如果A相为0，则说明A相先跳变，顺时针，counter++

            counter++;

        }else {//判断A相状态，如果A相为1，则说明A相后跳变，逆时针，counter--

            counter--;

        }

    }

    EXTI_ClearITPendingBit(EXTI_Line1);

}

//返会counter值(加入中断的关闭和打开，防止主程序在读取到一半时，数据又被拉到中断更改，导致读取到脏数据)
int32_t get_Num(void) {

    //关掉中断
    __disable_irq();
    int32_t temp = counter;
    //打开中断
    __enable_irq();
	
    return temp;

}
