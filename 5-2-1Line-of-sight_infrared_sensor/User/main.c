#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Counter_sensor.h"
#include <stdint.h>

int main(void) {

    OLED_Init();
    sensor_Init();

    while (1) {

        OLED_ShowNum(1,1,get_Counter(),5);

    }

}
