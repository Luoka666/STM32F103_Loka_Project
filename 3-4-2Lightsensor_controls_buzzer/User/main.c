#include "stm32f10x.h"// Device header
#include "delay.h"
#include "Buzzer.h"
#include "Lightsensor.h"

int main(void){
	
	Buzzer_Init();
	lightsensor_Init();
	
	uint8_t num = 0;
	
	while(1){ 
		
		num = readNumber();
		num ? Buzzer_Off() : Buzzer_On();
			
		
	}
	
}
