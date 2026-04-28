#include "stm32f10x.h"

/**
  * 函    数：USART串口初始化
  * 参    数：无
  * 返 回 值：无
  */
 void usart_Init(void) {

     //打开时钟
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

     GPIO_InitTypeDef s;
     s.GPIO_Mode = GPIO_Mode_AF_PP;
     s.GPIO_Speed = GPIO_Speed_50MHz;
     s.GPIO_Pin = GPIO_Pin_9;
     GPIO_Init(GPIOA, &s);


     //初始化
     USART_InitTypeDef p;
     p.USART_BaudRate = 9600;
     p.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不需要硬件控制流
     p.USART_Mode = USART_Mode_Tx;//配置为发送模式，单片机发送数据至电脑，串口助手显示
     p.USART_Parity = USART_Parity_No;
     p.USART_StopBits = USART_StopBits_1;
     p.USART_WordLength = USART_WordLength_8b;
     USART_Init(USART1, &p);
     USART_Cmd(USART1, ENABLE);//启用USART1外设

 }
/**
  * 函    数：温湿度数据发送
  * 参    数：uint8_t temp，uint8_t humi
  * 返 回 值：无
  */
void usart_send(uint8_t temp, uint8_t humi) {
    // 发送温度
    USART_SendData(USART1, temp);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

    // 发送湿度
    USART_SendData(USART1, humi);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}
void usart_send_str(uint8_t *str) {

     while(*str) {
         USART_SendData(USART1, *str++);
     }

 }
