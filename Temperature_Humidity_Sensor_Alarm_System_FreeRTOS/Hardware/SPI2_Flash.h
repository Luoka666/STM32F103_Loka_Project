#ifndef SPI2_FLASH_H
#define SPI2_FLASH_H

#include <stdint.h>

/* 这一层只负责 SPI2 总线，不认识“阈值”“扇区”等 Flash 业务。
 * 接线：PB12=CS，PB13=SCK，PB14=MISO，PB15=MOSI，模块使用 3.3V。
 * 当前总线仅由开机初始化和 StateMachine 任务使用，不能并发调用。
 */
void SPI2_Flash_Init(void);
void SPI2_Flash_Select(void);
uint8_t SPI2_Flash_Deselect(void);
uint8_t SPI2_Flash_ExchangeByte(uint8_t tx, uint8_t *rx);

#endif
