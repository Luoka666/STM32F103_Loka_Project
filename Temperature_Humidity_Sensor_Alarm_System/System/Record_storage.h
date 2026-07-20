//
// Created by 32663 on 2026/5/27.
//

#ifndef TEMPLE_PROJECT_RECORD_STORAGE_H
#define TEMPLE_PROJECT_RECORD_STORAGE_H

#define HISTORY_SIZE 4   //最大存四条数据

extern uint8_t history_temp[];
extern uint8_t history_humi[];
extern uint8_t history_count;
extern uint8_t write_index;

void history_add(uint8_t temp, uint8_t humi);

#endif //TEMPLE_PROJECT_RECORD_STORAGE_H
