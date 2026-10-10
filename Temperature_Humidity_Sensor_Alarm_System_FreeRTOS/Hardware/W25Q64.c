#include "FreeRTOS.h"
#include "task.h"
#include "W25Q64.h"
#include "SPI2_Flash.h"
#include "timer_delay.h"

/* Flash 指令层：把 SPI 字节组合成芯片手册规定的命令。 */
#define CMD_READ_ID       0x9FU
#define CMD_READ_STATUS1  0x05U
#define CMD_WRITE_ENABLE  0x06U
#define CMD_READ          0x03U
#define CMD_PAGE_PROGRAM  0x02U
#define CMD_SECTOR_ERASE  0x20U
#define CMD_WAKE_UP       0xABU
#define STATUS_BUSY       0x01U
#define STATUS_WEL        0x02U
#define FLASH_BUSY_LIMIT_MS  2000UL

static uint8_t flash_ready = 0;

static void Flash_PauseMs(uint16_t milliseconds) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        /* Flash 在芯片内部擦写时无需一直占 CPU，也不用关中断。
         * 每次查询 BUSY 后先释放 CS，再让其他任务运行。
         */
        vTaskDelay(pdMS_TO_TICKS(milliseconds));
    } else {
        /* 开机还没启动调度器，不能调用 vTaskDelay；TIM2 已由 main 初始化。 */
        Delay_ms_TIM(milliseconds);
    }
}

static W25Q64_Result Flash_Finish(uint8_t ok) {
    if (!SPI2_Flash_Deselect()) ok = 0;
    if (!ok) {
        /* 出错后重新准备 STM32 的 SPI 外设，清除残留的收发状态。
         * 不复位、不解锁、不整片擦除 Flash，避免更改未知保护设置。
         */
        SPI2_Flash_Init();
        return W25Q64_SPI_TIMEOUT;
    }
    return W25Q64_OK;
}

static uint8_t Flash_SendAddress(uint32_t address) {
    uint8_t ignored;
    /* 24 位地址按高字节在前发送，与 MCU 的内存大小端无关。 */
    return SPI2_Flash_ExchangeByte((uint8_t)(address >> 16), &ignored) &&
           SPI2_Flash_ExchangeByte((uint8_t)(address >> 8), &ignored) &&
           SPI2_Flash_ExchangeByte((uint8_t)address, &ignored);
}

static W25Q64_Result Flash_ReadStatus(uint8_t *status) {
    uint8_t ignored;
    uint8_t ok;
    SPI2_Flash_Select();
    ok = SPI2_Flash_ExchangeByte(CMD_READ_STATUS1, &ignored) &&
         SPI2_Flash_ExchangeByte(0xFFU, status);
    return Flash_Finish(ok);
}

static W25Q64_Result Flash_WaitReady(void) {
    uint32_t waits = 0;
    uint8_t status;
    W25Q64_Result result;
    for (;;) {
        result = Flash_ReadStatus(&status);
        if (result != W25Q64_OK) return result;
        if ((status & STATUS_BUSY) == 0U) return W25Q64_OK;
        if (waits++ >= FLASH_BUSY_LIMIT_MS) return W25Q64_BUSY_TIMEOUT;
        Flash_PauseMs(1);
    }
    /* 这是最多 2000 次 1ms 等待的保护上限；调度延迟会延长实际墙钟时间。
     * W25Q64JV 手册的 4KB 擦除最大值为 500ms，此处留有余量。
     */
}

static W25Q64_Result Flash_WriteEnable(void) {
    uint8_t ignored;
    uint8_t status;
    W25Q64_Result result = Flash_WaitReady();
    if (result != W25Q64_OK) return result;
    SPI2_Flash_Select();
    result = Flash_Finish(SPI2_Flash_ExchangeByte(CMD_WRITE_ENABLE, &ignored));
    if (result != W25Q64_OK) return result;
    result = Flash_ReadStatus(&status);
    if (result != W25Q64_OK) return result;
    /* 每次擦除/编程之前都要重新发 06h，不能只在初始化时发一次。
     * WEL 置位不代表目标区域一定未受保护，应用层仍需读回核对数据。
     */
    return (status & STATUS_WEL) ? W25Q64_OK : W25Q64_WRITE_DISABLED;
}

W25Q64_Result W25Q64_Init(uint32_t *jedec_id) {
    uint8_t ignored;
    uint8_t id[3];
    uint8_t ok;
    W25Q64_Result result;
    if (jedec_id == 0) return W25Q64_BAD_ARGUMENT;
    *jedec_id = 0;
    flash_ready = 0;
    SPI2_Flash_Init();
    Flash_PauseMs(10); /* 给上电及之前擦写后的状态恢复留时间。 */
    SPI2_Flash_Select();
    result = Flash_Finish(SPI2_Flash_ExchangeByte(CMD_WAKE_UP, &ignored));
    if (result != W25Q64_OK) return result;
    Flash_PauseMs(1); /* ABh 退出深度掉电后，等待再访问。 */
    result = Flash_WaitReady();
    if (result != W25Q64_OK) return result;

    SPI2_Flash_Select();
    ok = SPI2_Flash_ExchangeByte(CMD_READ_ID, &ignored) &&
         SPI2_Flash_ExchangeByte(0xFFU, &id[0]) &&
         SPI2_Flash_ExchangeByte(0xFFU, &id[1]) &&
         SPI2_Flash_ExchangeByte(0xFFU, &id[2]);
    result = Flash_Finish(ok);
    if (result != W25Q64_OK) return result;
    *jedec_id = ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
    if (*jedec_id != 0xEF4017UL && *jedec_id != 0xEF7017UL) {
        return W25Q64_UNSUPPORTED_ID;
    }
    flash_ready = 1;
    return W25Q64_OK;
}

W25Q64_Result W25Q64_Read(uint32_t address, uint8_t *data, uint16_t length) {
    uint8_t ignored;
    uint8_t ok;
    uint16_t i;
    W25Q64_Result result;
    if (data == 0 || length == 0 || address >= W25Q64_CAPACITY_BYTES ||
        length > W25Q64_CAPACITY_BYTES - address) return W25Q64_BAD_ARGUMENT;
    if (!flash_ready) return W25Q64_NOT_READY;
    result = Flash_WaitReady();
    if (result != W25Q64_OK) return result;
    SPI2_Flash_Select();
    ok = SPI2_Flash_ExchangeByte(CMD_READ, &ignored) && Flash_SendAddress(address);
    for (i = 0; ok && i < length; i++) {
        ok = SPI2_Flash_ExchangeByte(0xFFU, &data[i]);
    }
    return Flash_Finish(ok);
}

W25Q64_Result W25Q64_PageProgram(uint32_t address, const uint8_t *data, uint16_t length) {
    uint8_t ignored;
    uint8_t ok;
    uint16_t i;
    W25Q64_Result result;
    if (data == 0 || length == 0 || address >= W25Q64_CAPACITY_BYTES ||
        length > W25Q64_CAPACITY_BYTES - address ||
        length > W25Q64_PAGE_BYTES - (address % W25Q64_PAGE_BYTES)) {
        return W25Q64_BAD_ARGUMENT;
    }
    if (!flash_ready) return W25Q64_NOT_READY;
    result = Flash_WriteEnable();
    if (result != W25Q64_OK) return result;
    SPI2_Flash_Select();
    ok = SPI2_Flash_ExchangeByte(CMD_PAGE_PROGRAM, &ignored) && Flash_SendAddress(address);
    for (i = 0; ok && i < length; i++) {
        ok = SPI2_Flash_ExchangeByte(data[i], &ignored);
    }
    result = Flash_Finish(ok);
    if (result != W25Q64_OK) return result;
    return Flash_WaitReady(); /* CS 拉高才启动芯片内部编程，必须等它完成。 */
}

W25Q64_Result W25Q64_EraseSector(uint32_t address) {
    uint8_t ignored;
    W25Q64_Result result;
    uint8_t ok;
    if (address >= W25Q64_CAPACITY_BYTES || address % W25Q64_SECTOR_BYTES != 0) {
        return W25Q64_BAD_ARGUMENT;
    }
    if (!flash_ready) return W25Q64_NOT_READY;
    result = Flash_WriteEnable();
    if (result != W25Q64_OK) return result;
    SPI2_Flash_Select();
    ok = SPI2_Flash_ExchangeByte(CMD_SECTOR_ERASE, &ignored) && Flash_SendAddress(address);
    result = Flash_Finish(ok);
    if (result != W25Q64_OK) return result;
    return Flash_WaitReady();
}
