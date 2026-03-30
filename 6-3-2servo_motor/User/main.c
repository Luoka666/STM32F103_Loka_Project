#include "stm32f10x.h"                  // Device header
#include "servo.h" 
#include "Key.h"
#include "OLED.h"
#include "Delay.h"

float Angle = 0;

int main(void) {

    OLED_Init();
    servo_Init();
	Key_Init();
	OLED_ShowString(1,1,"Angle:");
	
    while(1){
		
		if(Key_GetNum() == 1){
		
			Angle+=30;
			
			if(Angle > 180){
			
				Angle = 0;
				
			}
			
			set_Angle(Angle);
			OLED_ShowNum(1,7,Angle,3);
			
		}
		
//		set_Angle(0);
//		Delay_ms(900);
//		set_Angle(180);
//		Delay_ms(900);
       



}


}
