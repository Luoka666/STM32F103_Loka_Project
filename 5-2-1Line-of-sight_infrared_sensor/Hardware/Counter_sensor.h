#ifndef _COUTER_SENSOR_H
#define _COUTER_SENSOR_H
#include <stdint.h>

void sensor_Init(void);
uint32_t get_Counter(void);
void EXTI0_IRQHandler(void);

#endif //TEMPLE_PROJECT_COUTER_SENSOR_H
