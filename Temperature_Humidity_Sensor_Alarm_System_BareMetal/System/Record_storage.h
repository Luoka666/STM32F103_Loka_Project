//
// Created by 32663 on 2026/5/27.
//

#ifndef TEMPLE_PROJECT_RECORD_STORAGE_H
#define TEMPLE_PROJECT_RECORD_STORAGE_H

#define HISTORY_SIZE 4   //最大存四条数据

extern uint8_t history_temp[];
extern uint8_t history_humi[];
extern uint8_t history_count; //记录队列此时的数据数，最大为4
extern uint8_t write_index; //永远指向最新数据的下一个位置，意味着在队列满后，则永远指向最旧的数据

void history_add(uint8_t temp, uint8_t humi);

#endif //TEMPLE_PROJECT_RECORD_STORAGE_H
