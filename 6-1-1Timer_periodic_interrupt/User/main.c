#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Timer.h"
uint16_t num = 0;
int main(void) {
    /*模块初始化*/
    OLED_Init(); //OLED初始化
	time_Init();

    //优先级分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    OLED_ShowString(1,1,"num:");
    OLED_ShowString(2,1,"cnt:");
    OLED_ShowString(3,1,"psc:");



    while (1) {
		
		OLED_ShowNum(1,5,num,5);//不断刷新显示NUMZ最新数值
        OLED_ShowNum(2,5,TIM_GetCounter(TIM2),5);//获取CNT的计数值
        OLED_ShowNum(3,5,TIM_GetPrescaler(TIM2),5);//获取PSC的值
		
    }
}

//中断函数
void TIM2_IRQHandler(void) {
	
    if (TIM_GetITStatus(TIM2,TIM_IT_Update) == SET) {

        num++;
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除标志位

    }

}
