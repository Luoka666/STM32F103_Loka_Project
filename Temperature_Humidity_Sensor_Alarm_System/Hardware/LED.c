#include "stm32f10x.h"
#include "delay.h"

/**
  * 函    数：LED初始化
  * 参    数：无
  * 返 回 值：无
  */
void LED_Init(void)
{
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟

    /*GPIO初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);						//将PA11脚初始化为推挽输出

    /*设置GPIO初始化后的默认电平*/
    GPIO_SetBits(GPIOA, GPIO_Pin_11);				//设置PA11
}

/**
  * 函    数：LED1开启
  * 参    数：无
  * 返 回 值：无
  */
void LED1_ON(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_11);		//设置PA11引脚为低电平
}

/**
  * 函    数：LED1关闭
  * 参    数：无
  * 返 回 值：无
  */
void LED1_OFF(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_11);		//设置PA11引脚为高电平
}

/**
  * 函    数：LED1状态翻转
  * 参    数：无
  * 返 回 值：无
  */
void LED1_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_1) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_6);					//则设置PA1引脚为高电平
    }
    else													//否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);					//则设置PA1引脚为低电平
    }
}

/**
  * 函    数：LED2开启
  * 参    数：无
  * 返 回 值：无
  */
void LED2_ON(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_2);		//设置PA2引脚为低电平
}

/**
  * 函    数：LED2关闭
  * 参    数：无
  * 返 回 值：无
  */
void LED2_OFF(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_2);		//设置PA2引脚为高电平
}

/**
  * 函    数：LED2状态翻转
  * 参    数：无
  * 返 回 值：无
  */
void LED2_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_2) == 0)		//获取输出寄存器的状态，如果当前引脚输出低电平
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_2);               		//则设置PA2引脚为高电平
    }
    else                                               		//否则，即当前引脚输出高电平
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);             		//则设置PA2引脚为低电平
    }
}

/**
  * 函    数：LED1闪烁
  * 参    数：无
  * 返 回 值：无
  */
// void LED1_flashing(void) {
//     //使用while1会让程序整体卡进led闪烁循环
//     while (1) {
//
//         LED1_ON();
//         Delay_ms(500);
//         LED1_OFF();
//         Delay_ms(500);
//     }
//
// }
