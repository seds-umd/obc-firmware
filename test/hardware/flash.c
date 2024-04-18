#include "flash.h"

#include "unity.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t sim_flash_buf[PICO_FLASH_SIZE_BYTES];

void flash_range_erase(uint32_t flash_offs, size_t count) {
    memset(sim_flash_buf + flash_offs, 0xFF, count);
}

void flash_range_program(uint32_t flash_offs, const uint8_t *data,
                         size_t count) {
    TEST_ASSERT_EQUAL_MESSAGE(0, flash_offs % 256,
                              "Flash offset must be 256 byte aligned");
    TEST_ASSERT_EQUAL_MESSAGE(0, count % 256,
                              "Flash program count must be a multiple of 256");

    // To simulate flash behavior correctly, we need to AND data with current
    // flash contents before writing it
    for (size_t i = 0; i < count; i++) {
        sim_flash_buf[flash_offs + i] = data[i] & sim_flash_buf[flash_offs + i];
    }
}

uint8_t sim_flash_read8(uint32_t addr) {
    return *(sim_flash_buf + addr);
}

uint16_t sim_flash_read16(uint32_t addr) {
    return *((uint16_t *) (sim_flash_buf + addr));
}

uint32_t sim_flash_read32(uint32_t addr) {
    return *((uint32_t *) (sim_flash_buf + addr));
}
