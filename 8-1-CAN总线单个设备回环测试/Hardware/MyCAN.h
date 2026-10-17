#ifndef __MYCAN_H
#define __MYCAN_H
#include <stdint.h>

void MYCAN_Init(void);
void MyCAN_SendMessage(uint32_t id, uint8_t *data, uint8_t len);
uint8_t MyCAN_ReceiveMessage(uint32_t *id, uint8_t *data, uint8_t *len);

#endif
