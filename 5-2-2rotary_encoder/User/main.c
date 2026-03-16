#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Encoder.h"
int main(void) {

    OLED_Init();
	
    //工程里有多个模块（比如电机、通信、传感器），每个模块都调用一次这个函数，效率降低。
    //通常系统中断分组只需要在 main 的开头配置一次即可。
    //配置优先级分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    encoder_Init();


    while(1){

        OLED_ShowSignedNum(2, 2,get_Num(), 5);

    }

}
