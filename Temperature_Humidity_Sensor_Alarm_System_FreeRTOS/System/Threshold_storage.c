#include "Threshold_storage.h"
#include <string.h>

/* 应用存储层不关心 SPI 寄存器，只关心“这份配置是不是完整有效”。
 * 手工组织字节，不直接把 C 结构体写进 Flash，避免结构体填充和大小端差异。
 *
 * 偏移  0~3 : 魔数 THR1，识别是不是我们的配置
 *       4   : 格式版本 1
 *       5/6 : 温度/湿度阈值
 *       7   : 保留字段，目前为 0
 *       8~11: 递增序号，低字节在前，用来选择更新的一份
 *       12/13: 前 12 字节的 CRC16-CCITT 校验，低字节在前
 *       14  : 提交标记 A5；最后单独写入，防止把写了一半的数据当成有效配置
 */
#define RECORD_BODY_BYTES 14U
#define RECORD_BYTES      15U
#define RECORD_COMMIT     0xA5U

static uint32_t jedec_id;
static W25Q64_Result flash_result;
static uint8_t have_record;
static uint8_t active_slot;
static uint8_t saved_temp, saved_humi;
static uint32_t saved_sequence;

static uint16_t Record_CRC16(const uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFFU;
    uint8_t i, bit;
    for (i = 0; i < length; i++) {
        crc ^= (uint16_t)((uint16_t)data[i] << 8);
        for (bit = 0; bit < 8; bit++) {
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1) ^ 0x1021U) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static uint32_t Record_Sequence(const uint8_t *record) {
    return (uint32_t)record[8] | ((uint32_t)record[9] << 8) |
           ((uint32_t)record[10] << 16) | ((uint32_t)record[11] << 24);
}

static uint8_t Record_Valid(const uint8_t *record) {
    uint16_t stored_crc = (uint16_t)(record[12] | ((uint16_t)record[13] << 8));
    return record[0] == 'T' && record[1] == 'H' && record[2] == 'R' && record[3] == '1' &&
           record[4] == 1U && record[5] <= 99U && record[6] <= 99U && record[7] == 0U &&
           record[14] == RECORD_COMMIT && stored_crc == Record_CRC16(record, 12);
}

static void Record_Build(uint8_t *record, uint8_t temp, uint8_t humi, uint32_t sequence) {
    uint16_t crc;
    record[0] = 'T'; record[1] = 'H'; record[2] = 'R'; record[3] = '1';
    record[4] = 1; record[5] = temp; record[6] = humi; record[7] = 0;
    record[8] = (uint8_t)sequence;
    record[9] = (uint8_t)(sequence >> 8);
    record[10] = (uint8_t)(sequence >> 16);
    record[11] = (uint8_t)(sequence >> 24);
    crc = Record_CRC16(record, 12);
    record[12] = (uint8_t)crc;
    record[13] = (uint8_t)(crc >> 8);
    record[14] = RECORD_COMMIT;
}

static uint32_t Slot_Address(uint8_t slot) {
    return slot == 0U ? THRESHOLD_SLOT_A_ADDRESS : THRESHOLD_SLOT_B_ADDRESS;
}

static ThresholdStore_Result Store_Scan(void) {
    uint8_t records[2][RECORD_BYTES];
    uint8_t valid_a, valid_b;
    uint32_t difference;
    /* 保存之前也重新扫描，处理“上次实际写完，但最终读回通信失败”的情况。
     * 任一扇区读取通信失败就拒绝擦写，不能把读不到误当作空白。
     */
    flash_result = W25Q64_Read(Slot_Address(0), records[0], RECORD_BYTES);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    flash_result = W25Q64_Read(Slot_Address(1), records[1], RECORD_BYTES);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    valid_a = Record_Valid(records[0]);
    valid_b = Record_Valid(records[1]);
    if (!valid_a && !valid_b) {
        have_record = 0;
        return THRESHOLD_STORE_DEFAULTS;
    }
    active_slot = (uint8_t)(valid_a ? 0U : 1U);
    /* 无符号减法允许序号从 FFFFFFFF 回绕到 0；两份相邻配置差值应很小。 */
    difference = Record_Sequence(records[1]) - Record_Sequence(records[0]);
    if (valid_a && valid_b && difference != 0U && difference < 0x80000000UL) active_slot = 1;
    saved_temp = records[active_slot][5];
    saved_humi = records[active_slot][6];
    saved_sequence = Record_Sequence(records[active_slot]);
    have_record = 1;
    return THRESHOLD_STORE_OK;
}

ThresholdStore_Result ThresholdStore_Init(uint8_t *temp, uint8_t *humi) {
    ThresholdStore_Result result;
    if (temp == 0 || humi == 0) return THRESHOLD_STORE_BAD_ARGUMENT;
    *temp = THRESHOLD_DEFAULT_TEMP;
    *humi = THRESHOLD_DEFAULT_HUMI;
    have_record = 0;
    flash_result = W25Q64_Init(&jedec_id);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    result = Store_Scan();
    if (result == THRESHOLD_STORE_OK) {
        *temp = saved_temp;
        *humi = saved_humi;
    }
    return result;
}

ThresholdStore_Result ThresholdStore_Save(uint8_t temp, uint8_t humi) {
    uint8_t record[RECORD_BYTES];
    uint8_t readback[RECORD_BYTES];
    uint8_t target_slot;
    uint8_t i;
    uint32_t sequence;
    uint32_t address;
    ThresholdStore_Result result;
    if (temp > 99U || humi > 99U) return THRESHOLD_STORE_BAD_ARGUMENT;
    /* 保存时再检测一次，接线修好后可直接按 K5 重试，不必只依赖开机结果。 */
    flash_result = W25Q64_Init(&jedec_id);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    result = Store_Scan();
    if (result != THRESHOLD_STORE_OK && result != THRESHOLD_STORE_DEFAULTS) return result;
    if (have_record && temp == saved_temp && humi == saved_humi) {
        return THRESHOLD_STORE_UNCHANGED; /* 没改值就不擦写，减少磨损。 */
    }

    target_slot = have_record ? (uint8_t)(1U - active_slot) : 0U;
    sequence = have_record ? saved_sequence + 1U : 1U;
    address = Slot_Address(target_slot);
    Record_Build(record, temp, humi, sequence);

    /* NOR Flash 编程只能把位从 1 改成 0；要把 0 改回 1，必须先擦除。
     * 永远擦“另一份”的扇区，当前有效配置先不动。这里只擦 4KB，不擦整片。
     */
    flash_result = W25Q64_EraseSector(address);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    flash_result = W25Q64_Read(address, readback, RECORD_BYTES);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    for (i = 0; i < RECORD_BYTES; i++) {
        if (readback[i] != 0xFFU) return THRESHOLD_STORE_VERIFY_ERROR;
    }

    /* 先写正文并读回核对，此时提交标记仍为 FF，重启不会采用这份配置。 */
    flash_result = W25Q64_PageProgram(address, record, RECORD_BODY_BYTES);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    flash_result = W25Q64_Read(address, readback, RECORD_BYTES);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    if (memcmp(record, readback, RECORD_BODY_BYTES) != 0 || readback[14] != 0xFFU) {
        return THRESHOLD_STORE_VERIFY_ERROR;
    }

    /* 正文完整后，最后单独提交一个字节，再核对整条记录。
     * 成功才更新内存缓存。异常掉电时通常可回退上一份；这不替代真实掉电测试
     * 或芯片规定的电源时序，不能宣称任何电源故障都能绝对保证不丢数据。
     */
    flash_result = W25Q64_PageProgram(address + RECORD_BODY_BYTES, &record[14], 1);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    flash_result = W25Q64_Read(address, readback, RECORD_BYTES);
    if (flash_result != W25Q64_OK) return THRESHOLD_STORE_FLASH_ERROR;
    if (!Record_Valid(readback) || memcmp(record, readback, RECORD_BYTES) != 0) {
        return THRESHOLD_STORE_VERIFY_ERROR;
    }
    have_record = 1;
    active_slot = target_slot;
    saved_temp = temp;
    saved_humi = humi;
    saved_sequence = sequence;
    return THRESHOLD_STORE_OK;
}

uint32_t ThresholdStore_GetJedecID(void) { return jedec_id; }
W25Q64_Result ThresholdStore_GetFlashResult(void) { return flash_result; }

const char *ThresholdStore_ResultText(ThresholdStore_Result result) {
    switch (result) {
        case THRESHOLD_STORE_OK: return "OK";
        case THRESHOLD_STORE_DEFAULTS: return "defaults";
        case THRESHOLD_STORE_UNCHANGED: return "unchanged";
        case THRESHOLD_STORE_FLASH_ERROR: return "Flash error";
        case THRESHOLD_STORE_VERIFY_ERROR: return "verify error";
        default: return "bad argument";
    }
}
