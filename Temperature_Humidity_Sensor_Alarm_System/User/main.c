#include "stm32f10x.h"
#include "dht11.h"
#include "oled.h"
#include "delay.h"
#include "USART.h"
#include "key.h"
uint8_t temp = 0;
uint8_t humi = 0;
uint8_t keyNum = 0;

//记录系统状态
typedef enum {
    STOP = 0,
    RUN
} SystemState;

SystemState State = STOP; //系统默认停止
SystemState last_State = STOP;//记录状态变化前的系统状态

int main(void) {
    DHT11_Init();
    OLED_Init();
    usart_Init();
    Key_Init();

    OLED_Clear(); // 清屏
    OLED_ShowString(1, 1, "DHT11 Monitor");

    while (1) {

        keyNum = Key_GetNum();
        //系统状态取反
        if (keyNum == 1) {

            State = !State;

        }
        //检测到状态变化，执行清屏
        if (State != last_State) {

            OLED_Clear();
            OLED_ShowString(1, 1, "DHT11 Monitor");
            last_State = State;//更新状态

        }


        if (State) {
            
            if (data_Check(&temp, &humi) == 1) {

                OLED_ShowString(2, 1, "Temp: ");
                OLED_ShowNum(2, 6, temp, 2);
                OLED_ShowChar(2, 8, 'C');

                OLED_ShowString(3, 1, "Humi: ");
                OLED_ShowNum(3, 6, humi, 2);
                OLED_ShowChar(3, 8, '%');

                OLED_ShowString(4, 1, "State: RUN  ");
                //串口发送
                usart_send(temp, humi);

            } else {
                
                OLED_Clear();//两种界面对应一种状态，需要加清屏，再绘画界面
				OLED_ShowString(1, 1, "DHT11 Monitor");
                OLED_ShowString(2, 1, "Read Failed!");
                OLED_ShowString(3, 1, "Check Wiring!");
                OLED_ShowString(4, 1, "Status: ERR ");

            }

            Delay_ms(2000);

        }else {

            OLED_ShowString(2, 1, "State: STOP");
            OLED_ShowString(3, 1, "Waiting");
            OLED_ShowString(4, 1, "Press Key");

        }

    }

}
