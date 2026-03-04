#include "stm32f10x.h"                  // Device header
void led_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef p;
	p.GPIO_Pin = 0x0003;
	p.GPIO_Speed = GPIO_Speed_50MHz;
	p.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA,&p);
	GPIO_SetBits(GPIOA,0x0003);
	
}

void led1_On(void){
	
	GPIO_WriteBit(GPIOA,0x0001,Bit_RESET);

}
void led1_Off(void){
	
	GPIO_WriteBit(GPIOA,0x0001,Bit_SET);

}

void led2_On(void){
	
	GPIO_WriteBit(GPIOA,0x0002,Bit_RESET);

}
void led2_Off(void){
	
	GPIO_WriteBit(GPIOA,0x0002,Bit_SET);

}
