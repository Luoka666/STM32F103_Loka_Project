#include "stm32f10x.h"
#include "Record_storage.h"

uint8_t history_temp[HISTORY_SIZE] = {0};
uint8_t history_humi[HISTORY_SIZE] = {0};
uint8_t history_count = 0;   // 实际存入数据条数
uint8_t write_index   = 0;   // 环形缓冲区写指针，满时最旧，即history_count == HISTORY_SIZE

/*
// 旧版：移位寄存器方式（O(n)，每次插入前移所有元素）
void history_add(uint8_t temp, uint8_t humi) {

    for (int i = 0; i < HISTORY_SIZE - 1; i++) {
        history_temp[i] = history_temp[i + 1];
        history_humi[i] = history_humi[i + 1];
    }

    history_temp[HISTORY_SIZE - 1] = temp;
    history_humi[HISTORY_SIZE - 1] = humi;

    if (history_count < HISTORY_SIZE) {
        history_count++;
    }
}
*/

// 新版：环形缓冲区（O(1)，覆盖写入，不移动数据，只标记最新数据应处在的位置，从而直接写入）
void history_add(uint8_t temp, uint8_t humi) {
	
    history_temp[write_index] = temp;
    history_humi[write_index] = humi;
    write_index = (write_index + 1) % HISTORY_SIZE;

    if (history_count < HISTORY_SIZE) {
        history_count++;
    }
	
}
