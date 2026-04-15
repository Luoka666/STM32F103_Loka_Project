#include "stm32f10x.h"
#include "dht11.h"
#include "oled.h"
#include "delay.h"
uint8_t temp = 0;
uint8_t humi = 0;


int main(void){

    DHT11_Init();
    OLED_Init();

    OLED_Clear();   // 清屏
    OLED_ShowString(1, 1, "DHT11 Monitor");

    while (1){

        if (data_Check(&temp, &humi) == 0)
        {
            OLED_ShowString(2, 1, "Temp: ");
            OLED_ShowNum(2, 6, temp, 2);
            OLED_ShowChar(2, 8, 'C');
            
            OLED_ShowString(3, 1, "Humi: ");
            OLED_ShowNum(3, 6, humi, 2);
            OLED_ShowChar(3, 8, '%');
            
            OLED_ShowString(4, 1, "Status: OK  ");
        }
        else
        {
            OLED_ShowString(2, 1, "Read Failed!");
            OLED_ShowString(3, 1, "Check Wiring!");
            OLED_ShowString(4, 1, "Status: ERR ");
        }
        
        Delay_ms(1000);
		
    }
	
}
