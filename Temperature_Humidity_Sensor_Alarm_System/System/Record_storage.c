#include "stm32f10x.h"
#include "Record_storage.h"

uint8_t history_temp[HISTORY_SIZE] = {0};
uint8_t history_humi[HISTORY_SIZE] = {0};
uint8_t history_count = 0;   //实际存入数据

// 使用了移位寄存器方式
void history_add(uint8_t temp,uint8_t humi) {

    for (int i = 0;i < HISTORY_SIZE - 1;i++) {

        history_temp[i] = history_temp[i + 1];   //挪动位置，腾出最后一位来存储最新数据
        history_humi[i] = history_humi[i + 1];

    }

    history_temp[HISTORY_SIZE - 1] = temp;   //添加新数据
    history_humi[HISTORY_SIZE - 1] = humi;

    if (history_count < HISTORY_SIZE) {
        history_count++;   //最高为4，在第一轮四个数据存满后，该变量一直为4.
    }

}
