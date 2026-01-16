#include "stm32f10x.h"                  // Device header
int main(void){
	//打开时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	//配置端口模式
	
	GPIO_InitTypeDef p;
	p.GPIO_Pin = GPIO_Pin_13;
	p.GPIO_Speed = GPIO_Speed_50MHz;//这是引脚的输出响应速度。
	p.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_Init(GPIOC,&p);
	//设置电平点灯
//	GPIO_SetBits(GPIOC,GPIO_Pin_13);//高电平不亮
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);//低电平灯亮
	while(1){
		
	}

	return 0;
}//最后一行空行
