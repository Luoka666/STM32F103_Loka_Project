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
        if (keyNum) {

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

                OLED_ShowString(1, 1, "DHT11 Monitor");
                OLED_ShowString(2, 1, "Read Failed!");
                OLED_ShowString(3, 1, "Check Wiring!");
                OLED_ShowString(4, 1, "Status: ERR ");

            }
            //按键失灵问题，不能从RUN状态成功转到STOP状态,在按键按下后，RUN状态下，系统可能正处于延时阶段，从而不能成功检测到按键。
            //将2000ms分成20次100ms，在延时中也检测按键是否按下
            // Delay_ms(2000);
            for (int i = 0; i < 20;i++) {

                Delay_ms(100);
				//处理完事件后，如果不重新通过函数查询硬件，程序就会一直认为“事件仍在发生”。
                keyNum = Key_GetNum(); // 必须重新获取按键状态！（防止旧的keyNum数据影响判断）,如果不按按键，则返回0
                if (keyNum) {

                    State = !State; 
                    break;

                }

            }

        }else {

            OLED_ShowString(2, 1, "State: STOP");
            OLED_ShowString(3, 1, "Waiting");
            OLED_ShowString(4, 1, "Press Key");

        }

    }

}
