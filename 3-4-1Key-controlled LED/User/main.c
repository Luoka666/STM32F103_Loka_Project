#include "stm32f10x.h"// Device header
#include "delay.h"
#include "LED.h"
#include "KEY.h"
int main(void){
	led_Init();
	key_Init();
	uint8_t num = 0;
	while(1){
		num = getNum();
		if(num == 1){
			//判断是否为1（1则表示灯为关闭状态）
			if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_0) == 1){
				led1_On();
			}else{
				led1_Off();
			}
		}
		if(num == 2){
			if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1) == 1){
				led2_On();
			}else{
				led2_Off();
			}
		}

	}                       
	
}
