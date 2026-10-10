#ifndef THRESHOLD_STORAGE_H
#define THRESHOLD_STORAGE_H

#include <stdint.h>
#include "W25Q64.h"

#define THRESHOLD_DEFAULT_TEMP 40U
#define THRESHOLD_DEFAULT_HUMI 60U
/* 两个扇区专用于阈值；以后加历史存储必须从 0x002000 之后另行分区。
 * 当前保存会擦除这两个保留扇区之一，不能在里面放其他数据。
 */
#define THRESHOLD_SLOT_A_ADDRESS 0x000000UL
#define THRESHOLD_SLOT_B_ADDRESS 0x001000UL

typedef enum {
    THRESHOLD_STORE_OK = 0,
    THRESHOLD_STORE_DEFAULTS,
    THRESHOLD_STORE_UNCHANGED,
    THRESHOLD_STORE_FLASH_ERROR,
    THRESHOLD_STORE_VERIFY_ERROR,
    THRESHOLD_STORE_BAD_ARGUMENT
} ThresholdStore_Result;

/* main 在 TIM2 初始化后、调度器启动前调用。读取失败也会输出默认值。
 * 只读配置，不自动写入默认值，避免每次启动都擦写 Flash。
 */
ThresholdStore_Result ThresholdStore_Init(uint8_t *temp, uint8_t *humi);
/* 只由 StateMachine 任务在 K5 返回时调用，范围沿用菜单的 0~99。
 * 同时保存温度和湿度；成功/未变化可返回，失败应留在设置页重试。
 */
ThresholdStore_Result ThresholdStore_Save(uint8_t temp, uint8_t humi);
uint32_t ThresholdStore_GetJedecID(void);
W25Q64_Result ThresholdStore_GetFlashResult(void);
const char *ThresholdStore_ResultText(ThresholdStore_Result result);

#endif
