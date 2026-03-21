#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Timer.h"
uint16_t num = 0;
int main(void) {

    timer_Init();
    OLED_Init();

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    OLED_ShowString(1,1,"num:");
    OLED_ShowString(2,1,"cnt:");

    while(1){

        OLED_ShowNum(1,5,num,5);
        OLED_ShowNum(2,5,TIM_GetCounter(TIM2),5);
        
    }

}
//中断函数
void TIM2_IRQHandler(void) {

    if (TIM_GetFlagStatus(TIM2,TIM_FLAG_Update) == SET) {

        num++;
		TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update);
    }

}
