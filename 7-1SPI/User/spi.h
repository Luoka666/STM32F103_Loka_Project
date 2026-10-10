#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"

void Flash_SPI_Init(void);
uint8_t Flash_ReadID(uint32_t *id);


#endif
