#ifndef W25Q64_H
#define W25Q64_H

#include <stdint.h>

#define W25Q64_CAPACITY_BYTES  (8UL * 1024UL * 1024UL)
#define W25Q64_SECTOR_BYTES    4096UL
#define W25Q64_PAGE_BYTES      256UL

typedef enum {
    W25Q64_OK = 0,
    W25Q64_BAD_ARGUMENT,
    W25Q64_NOT_READY,
    W25Q64_SPI_TIMEOUT,
    W25Q64_BUSY_TIMEOUT,
    W25Q64_WRITE_DISABLED,
    W25Q64_UNSUPPORTED_ID
} W25Q64_Result;

/* 初始化/读取 JEDEC ID 不擦写任何数据。只接受 EF4017 / EF7017。
 * 未识别芯片时拒绝擦写，不把“读到一些字节”当作连接正确。
 */
W25Q64_Result W25Q64_Init(uint32_t *jedec_id);
W25Q64_Result W25Q64_Read(uint32_t address, uint8_t *data, uint16_t length);
/* 页编程要求地址+长度不跨 256 字节页；调用者负责先擦除目标区域。 */
W25Q64_Result W25Q64_PageProgram(uint32_t address, const uint8_t *data, uint16_t length);
/* 只允许 4KB 对齐的扇区地址；没有提供危险的“整片擦除”接口。 */
W25Q64_Result W25Q64_EraseSector(uint32_t address);

#endif
