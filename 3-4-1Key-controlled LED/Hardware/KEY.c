#include "stm32f10x.h"                  // Device header
#include "delay.h"

void key_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef p;
	p.GPIO_Pin = 0x0802;
	p.GPIO_Speed = GPIO_Speed_50MHz;
	p.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB,&p);

}

uint8_t getNum(void){
	uint8_t num = 0;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0){
		Delay_ms(20);//按下消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0);//松手后跳出循环
		Delay_ms(20);//松开消抖
		num = 1;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0){
		Delay_ms(20);//按下消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0);//松手后跳出循环
		Delay_ms(20);//松开消抖
		num = 2;
	}
	return num;
}
