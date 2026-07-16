#include "stm32f10x.h"                  // Device header
#include "Delay.h"

int main(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_Out_PP;
    p.GPIO_Pin = GPIO_Pin_12;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &p);
	while(1){
		
	GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_RESET);//低电平驱动（响）
		Delay_ms(500);
	GPIO_WriteBit(GPIOA,GPIO_Pin_12,Bit_SET);//关
		Delay_ms(500);
	
	
		
	}

}
