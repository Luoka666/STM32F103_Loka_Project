#include "stm32f10x.h"                  // Device header

void Buzzer_Init(void){ 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef p;
	p.GPIO_Mode = GPIO_Mode_Out_PP;
	p.GPIO_Pin = GPIO_Pin_0;
	p.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&p);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);//默认为低电平，改为高电平

}

void Buzzer_On(void){
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	
}

void Buzzer_Off(void){
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	
}

