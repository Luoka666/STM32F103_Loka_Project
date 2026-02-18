#include "stm32f10x.h"// Device header
#include "delay.h"
int main(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef P;
	P.GPIO_Mode = GPIO_Mode_Out_PP;
	P.GPIO_Pin = GPIO_Pin_0;
	P.GPIO_Speed =  GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&P);
	while(1){
		
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
		Delay_ms(500);
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
		Delay_ms(500);
	
	}
	
}
