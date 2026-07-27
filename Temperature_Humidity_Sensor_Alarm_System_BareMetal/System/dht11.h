#ifndef __DHT11_H
#define __DHT11_H

void DHT11_Init(void);
uint8_t data_Check(uint8_t *temp, uint8_t *humi);

#endif
