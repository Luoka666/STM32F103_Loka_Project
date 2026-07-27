#include "stm32f10x.h"                  // Device header


void Buzzer_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef p;
    p.GPIO_Mode = GPIO_Mode_Out_PP;
    p.GPIO_Pin = GPIO_Pin_12;
    p.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &p);
}

void Buzzer_on(void) {
    GPIO_WriteBit(GPIOA,GPIO_Pin_12, Bit_RESET); //低电平驱动（响）
}

void Buzzer_off(void) {
    GPIO_WriteBit(GPIOA,GPIO_Pin_12, Bit_SET); //关
}
