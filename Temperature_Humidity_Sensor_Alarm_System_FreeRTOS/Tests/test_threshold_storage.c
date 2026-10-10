/* Host-only tests: compile real W25Q64.c and Threshold_storage.c against an
 * emulated SPI Flash. They do not claim to test wires, power rails or MCU timing.
 * The mock implements NOR 1->0 programming, WEL, BUSY and CS transaction edges.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "Threshold_storage.h"
#include "SPI2_Flash.h"
#include "task.h"

static uint8_t memory[W25Q64_CAPACITY_BYTES];
static uint8_t selected, command, address_bytes, wel;
static uint32_t address, index_in_command;
static uint8_t program[256];
static uint16_t program_length;
static uint32_t device_id = 0xEF4017UL;
static uint32_t erase_count, program_count, elapsed_ms, delays_in_tasks;
static uint16_t busy_cycles;
static int scheduler_running, protected_area, refuse_wel, busy_forever;
static int abort_body, abort_commit, lose_final_read, fail_next_read;
static int corrupt_program, fail_next_byte, fail_next_deselect;
static unsigned int passed;

static void Pause(uint32_t ms) {
    elapsed_ms += ms;
    busy_cycles = ms >= busy_cycles ? 0 : (uint16_t)(busy_cycles - ms);
}
int xTaskGetSchedulerState(void) {
    return scheduler_running ? taskSCHEDULER_RUNNING : taskSCHEDULER_NOT_STARTED;
}
void vTaskDelay(TickType_t ticks) { delays_in_tasks++; Pause(ticks); }
void Delay_ms_TIM(uint16_t ms) { Pause(ms); }

void SPI2_Flash_Init(void) { selected = 0; }
void SPI2_Flash_Select(void) {
    assert(!selected);
    selected = 1; command = 0; address = 0; address_bytes = 0;
    index_in_command = 0; program_length = 0;
}

uint8_t SPI2_Flash_ExchangeByte(uint8_t tx, uint8_t *rx) {
    assert(selected && rx != NULL);
    *rx = 0xFF;
    if (fail_next_byte) { fail_next_byte = 0; return 0; }
    if (command == 0) {
        command = tx;
        if (command == 0x03 && fail_next_read) { fail_next_read = 0; return 0; }
        return 1;
    }
    if (command == 0x05) {
        *rx = (uint8_t)((wel ? 2 : 0) | ((busy_cycles || busy_forever) ? 1 : 0));
    } else if (command == 0x9F) {
        assert(index_in_command < 3);
        *rx = (uint8_t)(device_id >> (16U - index_in_command * 8U));
        index_in_command++;
    } else if (command == 0x03 || command == 0x02 || command == 0x20) {
        if (address_bytes < 3) {
            address = (address << 8) | tx;
            address_bytes++;
        } else if (command == 0x03) {
            assert(address + index_in_command < sizeof(memory));
            *rx = memory[address + index_in_command++];
        } else if (command == 0x02) {
            assert(program_length < sizeof(program));
            program[program_length++] = tx;
        }
    }
    return 1;
}

uint8_t SPI2_Flash_Deselect(void) {
    uint16_t i, count = program_length;
    uint8_t ok = 1;
    assert(selected);
    selected = 0;
    if (command == 0x06 && !busy_cycles && !busy_forever && !refuse_wel) wel = 1;
    if (command == 0x20 && address_bytes == 3 && wel) {
        assert(address % 4096 == 0 && address + 4096 <= sizeof(memory));
        erase_count++;
        if (!protected_area) memset(memory + address, 0xFF, 4096);
        wel = 0; busy_cycles = 3;
    }
    if (command == 0x02 && address_bytes == 3 && program_length && wel) {
        program_count++;
        if (abort_body && address % 4096 == 0) { count = 5; abort_body = 0; ok = 0; }
        if (abort_commit && address % 4096 == 14) { count = 0; abort_commit = 0; ok = 0; }
        if (!protected_area) {
            for (i = 0; i < count; i++) {
                assert(address + i < sizeof(memory));
                memory[address + i] &= program[i];
            }
            if (corrupt_program && address % 4096 == 0) {
                memory[address + 5] ^= 1U; corrupt_program = 0;
            }
        }
        if (lose_final_read && address % 4096 == 14) {
            lose_final_read = 0; fail_next_read = 1;
        }
        wel = 0; busy_cycles = 2;
    }
    if (fail_next_deselect) { fail_next_deselect = 0; ok = 0; }
    return ok;
}

static void Fresh(void) {
    memset(memory, 0xFF, sizeof(memory));
    selected = 0; wel = 0; busy_cycles = 0;
    device_id = 0xEF4017UL; erase_count = program_count = 0;
    elapsed_ms = delays_in_tasks = 0; scheduler_running = 0;
    protected_area = refuse_wel = busy_forever = 0;
    abort_body = abort_commit = lose_final_read = fail_next_read = 0;
    corrupt_program = fail_next_byte = fail_next_deselect = 0;
}
static void CheckBoot(ThresholdStore_Result expected, uint8_t temp, uint8_t humi) {
    uint8_t actual_temp = 0, actual_humi = 0;
    scheduler_running = 0;
    assert(ThresholdStore_Init(&actual_temp, &actual_humi) == expected);
    assert(actual_temp == temp && actual_humi == humi && !selected);
    scheduler_running = 1;
}
static void Pass(const char *name) { printf("PASS: %s\n", name); passed++; }

/* Independent fixture encoder for boundary/corruption tests. */
static void Fixture(uint32_t at, uint32_t sequence, uint8_t temp, uint8_t humi) {
    uint8_t *p = memory + at;
    uint16_t crc = 0xFFFFU;
    unsigned int i, b;
    memcpy(p, "THR1", 4); p[4] = 1; p[5] = temp; p[6] = humi; p[7] = 0;
    for (i = 0; i < 4; i++) p[8 + i] = (uint8_t)(sequence >> (i * 8));
    for (i = 0; i < 12; i++) {
        crc ^= (uint16_t)((uint16_t)p[i] << 8);
        for (b = 0; b < 8; b++) {
            uint16_t high = crc & 0x8000U;
            crc = (uint16_t)(crc << 1);
            if (high) crc ^= 0x1021U;
        }
    }
    p[12] = (uint8_t)crc; p[13] = (uint8_t)(crc >> 8); p[14] = 0xA5;
}

int main(void) {
    uint32_t writes, id;
    uint8_t bytes[2] = {0x12, 0x34};

    Fresh(); CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    assert(erase_count == 0 && program_count == 0);
    assert(ThresholdStore_GetJedecID() == 0xEF4017UL);
    Pass("blank Flash uses defaults without writing");

    memory[8192] = 0x5A;
    assert(ThresholdStore_Save(35, 70) == THRESHOLD_STORE_OK);
    assert(memory[0] == 'T' && memory[14] == 0xA5 && memory[8192] == 0x5A);
    CheckBoot(THRESHOLD_STORE_OK, 35, 70);
    assert(delays_in_tasks > 0);
    Pass("save, reboot, CS release, task yielding and partition boundary");

    writes = erase_count;
    assert(ThresholdStore_Save(35, 70) == THRESHOLD_STORE_UNCHANGED);
    assert(erase_count == writes);
    Pass("unchanged values do not erase");

    assert(ThresholdStore_Save(36, 71) == THRESHOLD_STORE_OK);
    assert(memory[4096] == 'T' && memory[0] == 'T');
    CheckBoot(THRESHOLD_STORE_OK, 36, 71);
    assert(ThresholdStore_Save(37, 72) == THRESHOLD_STORE_OK);
    CheckBoot(THRESHOLD_STORE_OK, 37, 72);
    Pass("alternate A/B and choose newest valid record");

    memory[5] ^= 1U;
    CheckBoot(THRESHOLD_STORE_OK, 36, 71);
    memory[4096 + 12] ^= 1U;
    CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    Pass("bad CRC falls back to old record or defaults");

    Fresh(); CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    assert(ThresholdStore_Save(30, 50) == THRESHOLD_STORE_OK);
    abort_body = 1;
    assert(ThresholdStore_Save(31, 51) == THRESHOLD_STORE_FLASH_ERROR);
    CheckBoot(THRESHOLD_STORE_OK, 30, 50);
    assert(memory[4096 + 14] == 0xFF);
    Pass("interrupted body write retains old configuration");

    abort_commit = 1;
    assert(ThresholdStore_Save(32, 52) == THRESHOLD_STORE_FLASH_ERROR);
    CheckBoot(THRESHOLD_STORE_OK, 30, 50);
    Pass("missing commit marker retains old configuration");

    lose_final_read = 1;
    assert(ThresholdStore_Save(33, 53) == THRESHOLD_STORE_FLASH_ERROR);
    writes = erase_count;
    assert(ThresholdStore_Save(33, 53) == THRESHOLD_STORE_UNCHANGED);
    assert(erase_count == writes);
    CheckBoot(THRESHOLD_STORE_OK, 33, 53);
    Pass("successful commit with lost readback is reconciled on retry");

    protected_area = 1;
    assert(ThresholdStore_Save(34, 54) == THRESHOLD_STORE_VERIFY_ERROR);
    protected_area = 0; CheckBoot(THRESHOLD_STORE_OK, 33, 53);
    Pass("write protection cannot produce false success");

    corrupt_program = 1;
    assert(ThresholdStore_Save(34, 54) == THRESHOLD_STORE_VERIFY_ERROR);
    CheckBoot(THRESHOLD_STORE_OK, 33, 53);
    Pass("body readback mismatch prevents commit");

    refuse_wel = 1;
    assert(ThresholdStore_Save(34, 54) == THRESHOLD_STORE_FLASH_ERROR);
    assert(ThresholdStore_GetFlashResult() == W25Q64_WRITE_DISABLED);
    refuse_wel = 0;
    Pass("WEL is checked before erasing/programming");

    Fresh(); device_id = 0xEF4018UL;
    CheckBoot(THRESHOLD_STORE_FLASH_ERROR, 40, 60);
    assert(ThresholdStore_Save(35, 70) == THRESHOLD_STORE_FLASH_ERROR);
    assert(erase_count == 0 && ThresholdStore_GetFlashResult() == W25Q64_UNSUPPORTED_ID);
    device_id = 0;
    CheckBoot(THRESHOLD_STORE_FLASH_ERROR, 40, 60);
    Pass("unsupported or missing ID cannot erase data");

    Fresh(); device_id = 0xEF7017UL;
    CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    Pass("W25Q64 EF7017 variant accepted");

    Fresh(); busy_forever = 1;
    CheckBoot(THRESHOLD_STORE_FLASH_ERROR, 40, 60);
    assert(ThresholdStore_GetFlashResult() == W25Q64_BUSY_TIMEOUT && elapsed_ms < 2100);
    Pass("BUSY timeout is bounded");

    Fresh(); fail_next_byte = 1;
    CheckBoot(THRESHOLD_STORE_FLASH_ERROR, 40, 60);
    assert(ThresholdStore_GetFlashResult() == W25Q64_SPI_TIMEOUT && !selected);
    fail_next_deselect = 1;
    CheckBoot(THRESHOLD_STORE_FLASH_ERROR, 40, 60);
    Pass("SPI errors propagate and release CS");

    Fresh(); CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    writes = erase_count;
    assert(ThresholdStore_Save(100, 60) == THRESHOLD_STORE_BAD_ARGUMENT);
    assert(ThresholdStore_Save(40, 255) == THRESHOLD_STORE_BAD_ARGUMENT);
    assert(W25Q64_PageProgram(255, bytes, 2) == W25Q64_BAD_ARGUMENT);
    assert(W25Q64_Read(W25Q64_CAPACITY_BYTES - 1, bytes, 2) == W25Q64_BAD_ARGUMENT);
    assert(W25Q64_EraseSector(1) == W25Q64_BAD_ARGUMENT);
    assert(W25Q64_Init(NULL) == W25Q64_BAD_ARGUMENT);
    assert(ThresholdStore_Init(NULL, bytes) == THRESHOLD_STORE_BAD_ARGUMENT);
    assert(erase_count == writes);
    Pass("range, page boundary, sector alignment and null checks");

    assert(ThresholdStore_Save(0, 99) == THRESHOLD_STORE_OK);
    CheckBoot(THRESHOLD_STORE_OK, 0, 99);
    Pass("threshold endpoints 0 and 99 persist");

    Fresh(); Fixture(0, 0xFFFFFFFFUL, 20, 40); Fixture(4096, 0, 21, 41);
    CheckBoot(THRESHOLD_STORE_OK, 21, 41);
    assert(ThresholdStore_Save(22, 42) == THRESHOLD_STORE_OK);
    CheckBoot(THRESHOLD_STORE_OK, 22, 42);
    Pass("sequence rollover chooses newest record");

    Fresh(); Fixture(0, 1, 100, 60); Fixture(4096, 2, 40, 100);
    CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    Fixture(0, 3, 25, 55); memory[4] = 2;
    CheckBoot(THRESHOLD_STORE_DEFAULTS, 40, 60);
    Pass("invalid thresholds and record version rejected");

    Fresh(); Fixture(0, 1, 20, 40); Fixture(4096, 2, 21, 41);
    CheckBoot(THRESHOLD_STORE_OK, 21, 41);
    fail_next_read = 1;
    assert(ThresholdStore_Save(22, 42) == THRESHOLD_STORE_FLASH_ERROR);
    assert(erase_count == 0);
    CheckBoot(THRESHOLD_STORE_OK, 21, 41);
    Pass("scan communication failure never erases an unknown slot");

    assert(W25Q64_Init(&id) == W25Q64_OK && id == 0xEF4017UL);
    printf("All %u host tests passed. Hardware validation is still required.\n", passed);
    return 0;
}
