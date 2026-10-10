#ifndef HOST_FREERTOS_H
#define HOST_FREERTOS_H
/* 仅供电脑上的 Flash 模拟测试；不加入 Keil 固件和 CLionIndex。 */
#include <stdint.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#endif
