#include "unity.h"

#include "hardware/flash.h"

#include <string.h>

void test_flash() {
    flash_range_erase(0, PICO_FLASH_SIZE_BYTES);

    for (int i = 0; i < PICO_FLASH_SIZE_BYTES; i++) {
        TEST_ASSERT_EQUAL_UINT8(0xFF, sim_flash_buf[i]);
    }

    uint8_t buf_zeros[256];
    memset(buf_zeros, 0, 256);

    uint8_t buf_ones[256];
    memset(buf_ones, 0xFF, 256);

    // Write all zeros
    flash_range_program(0x123400, buf_zeros, 256);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(buf_zeros, sim_flash_buf + 0x123400, 256);

    // Write ones - should not change
    flash_range_program(0x123400, buf_ones, 256);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(buf_zeros, sim_flash_buf + 0x123400, 256);

    // Erase to go back to ones
    flash_range_erase(0x123400, 256);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(buf_ones, sim_flash_buf + 0x123400, 256);

    uint8_t buf[256];
    memset(buf, 0xFF, 256);

    buf[72] = 0b11111110;
    flash_range_program(0x432100, buf, 256);

    buf[72] = 0b01111111;
    flash_range_program(0x432100, buf, 256);

    TEST_ASSERT_EQUAL_HEX8(0b01111110, *(sim_flash_buf + 0x432100 + 72));
}
