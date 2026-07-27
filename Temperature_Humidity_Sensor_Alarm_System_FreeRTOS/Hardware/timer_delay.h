#ifndef __TIMER_DELAY_H
#define __TIMER_DELAY_H

#include "stm32f10x.h"

void TIM2_Delay_Init(void);
void Delay_us_TIM(uint16_t xus);
void Delay_ms_TIM(uint16_t xms);

#endif
