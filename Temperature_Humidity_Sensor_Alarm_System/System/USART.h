//
// Created by 32663 on 2026/4/23.
//
#include <stdint.h>

#ifndef TEMPLE_PROJECT_USART_H
#define TEMPLE_PROJECT_USART_H

void usart_Init(void);
void usart_send(uint8_t temp, uint8_t humi);

#endif //TEMPLE_PROJECT_USART_H
