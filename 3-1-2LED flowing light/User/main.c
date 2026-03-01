#include "stm32f10x.h"// Device header
#include "delay.h"
int main(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef P;
	P.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	//GPIO_Pin_0 | GPIO_Pin_1
	P.GPIO_Pin = 0x00FF;// 0000 0000 0000 0001 | 0000 0000 0000 0010 == 0000 0000 0000 0011
	P.GPIO_Speed =  GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&P);
	uint16_t pins[] = {GPIO_Pin_0,GPIO_Pin_1,GPIO_Pin_2,GPIO_Pin_3,GPIO_Pin_4,GPIO_Pin_5,GPIO_Pin_6,GPIO_Pin_7};
	while(1){ 
		//用GPIO_ResetBits && GPIO_SetBits
//		for(int i  = 0; i < 8; i++){
//			
//			//关闭前一个灯
//			GPIO_ResetBits(GPIOA,pins[i - 1]);
//			//打开LED
//			GPIO_SetBits(GPIOA,pins[i]);
//			Delay_ms(200);
//			if( i == 7)//最后循环到7时需要在跳出for循环之前置GPIO_Pin_7为0，关闭led灯
//				GPIO_ResetBits(GPIOA,pins[i]);
//			
//		}
		//用GPIO_WriteBit
//		for(int i = 0; i < 8; i++){
//			
//			GPIO_WriteBit(GPIOA,pins[i - 1],Bit_RESET);
//			GPIO_WriteBit(GPIOA,pins[i],Bit_SET);
//			Delay_ms(200);
//			if(i == 7)
//				GPIO_WriteBit(GPIOA,pins[i],Bit_RESET);
//		
//		}
		for(int i = 0; i < 8; i++) {
			// 1. 点亮当前引脚
			GPIO_WriteBit(GPIOA, pins[i], Bit_SET);
			Delay_ms(200);
			
			// 2. 熄灭当前引脚（为下一次循环点亮下一个引脚做准备）
			GPIO_WriteBit(GPIOA, pins[i], Bit_RESET);
			
			// 这种写法简单清晰：亮 -> 延时 -> 灭 -> 循环下一个
		}
		
		
//		GPIO_ResetBits(GPIOA,GPIO_Pin_7);
//		GPIO_SetBits(GPIOA,GPIO_Pin_0);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
//		GPIO_SetBits(GPIOA,GPIO_Pin_1);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
//		GPIO_SetBits(GPIOA,GPIO_Pin_2);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
//		GPIO_SetBits(GPIOA,GPIO_Pin_3);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_3);
//		GPIO_SetBits(GPIOA,GPIO_Pin_4);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_4);
//		GPIO_SetBits(GPIOA,GPIO_Pin_5);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_5);
//		GPIO_SetBits(GPIOA,GPIO_Pin_6);
//		Delay_ms(200);
//		GPIO_ResetBits(GPIOA,GPIO_Pin_6);
//		GPIO_SetBits(GPIOA,GPIO_Pin_7);
//		Delay_ms(200);
//	
	}
	
}
