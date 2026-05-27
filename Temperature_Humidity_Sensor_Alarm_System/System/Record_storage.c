#include "stm32f10x.h"
#define history_size 4   //最大存四条数据

uint8_t history_temp[history_size] = {0};
uint8_t history_humi[history_size] = {0};
// 构建环形缓冲区进行存储
void history_add(void) {



}
