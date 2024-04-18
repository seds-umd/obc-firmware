#include "unity.h"

#include "bl_common.h"
#include "updater.h"

#include "hardware/flash.h"

#include <string.h>

// Function declarations to use internal functions of updater - a bit hacky
void set_update_status(uint16_t addr);
uint16_t count_remaining_chunks();

void test_update_status() {
    // Start with empty flash
    flash_range_erase(0, PICO_FLASH_SIZE_BYTES);

    // Write update size
    uint32_t buf[64];
    memset(buf, 0xFF, 256);

    buf[0] = 128 * 77;  // 2 loops of 32 + 13 remainder
    buf[1] = 0;

    flash_range_program(BL_UPDATE_HEADER_UPDATE_SIZE, (uint8_t *)buf, 256);

    TEST_ASSERT_EQUAL(77, count_remaining_chunks());

    for (int i = 0; i < 12; i++) set_update_status(i);
    TEST_ASSERT_EQUAL(77 - 12, count_remaining_chunks());

    for (int i = 12; i < 77; i++) set_update_status(i);
    TEST_ASSERT_EQUAL(0, count_remaining_chunks());
}

void test_updater() { test_update_status(); }
