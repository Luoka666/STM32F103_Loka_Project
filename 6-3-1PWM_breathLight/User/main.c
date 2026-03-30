#include "stm32f10x.h"                  // Device header
#include "breath_Light.h"
#include "Delay.h"
#include "OLED.h"
uint16_t i;
uint32_t s;
int main(void) {

    breathLight();
    OLED_Init();
    OLED_ShowChar(1,4,'%');//显示占空比
    while(1){

        for (i = 0;i < 1000;i++) {

            TIM_SetCompare1(TIM2,i);
             s = (i * 100) / 999;
            OLED_ShowNum(1,1,s,3);
            Delay_ms(1);    // 0 -- 999,共1000级，1000 * 1ms，共1S，时间合适，一秒慢亮，一秒慢灭。


        }
        for (i = 0;i < 1000;i++) {

            TIM_SetCompare1(TIM2,999 - i);
            s = ((999 - i) * 100) / 999;
            OLED_ShowNum(1,1,s,3);
            Delay_ms(1);

        }

    }

}
