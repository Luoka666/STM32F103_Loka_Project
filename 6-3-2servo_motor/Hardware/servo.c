#include "stm32f10x.h"                  // Device header
#include "breath_Light.h"

void servo_Init(void){

	breathLight();

}
void set_Angle(float x){

	TIM_SetCompare2(TIM2,x / 180 * 2000 + 500);

}
