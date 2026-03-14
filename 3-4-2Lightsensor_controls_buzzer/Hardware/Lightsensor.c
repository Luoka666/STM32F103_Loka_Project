#include "stm32f10x.h"   
#include "delay.h"
void lightsensor_Init(void){
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef p;
	p.GPIO_Mode =  GPIO_Mode_IPU ;//平常状态为高电平，传感器触发后为低电平
	p.GPIO_Pin = GPIO_Pin_5;
	p.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&p);

}

uint8_t readNumber(void){
	uint8_t num = 0;
	//遮光
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0){
	//消抖
		Delay_ms(20);
	//赋值
	// 二次验证还是低电平 → 确认触发，返回0
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0){
            return 0;
        }
	// 非低电平/电平抖动 → 返回1（蜂鸣器关闭）
    return 1;
	}

	
	
	
}
