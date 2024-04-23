#pragma once

#include "hardware/flash.h"

#include <string.h>

// Program flash section addresses and sizes

#define BL_BOOT2_START 0x000000
#define BL_BOOT2_SIZE 0x100

#define BL_BOOTLOADER_START 0x000100
#define BL_BOOTLOADER_SIZE 0x3F00

#define BL_APP_HEADER_START 0x004000
#define BL_APP_HEADER_SIZE 0x1000

#define BL_UPDATE_HEADER_START 0x005000
#define BL_UPDATE_HEADER_SIZE 0x1000

#define BL_CONFIG_START 0x006000
#define BL_CONFIG_SIZE 0xFA000

#define BL_APP_START 0x080000
#define BL_APP_SIZE 0x80000

#define BL_UPDATE_START 0x100000
#define BL_UPDATE_SIZE 0x80000

// App header layout
#define BL_APP_HEADER_APP_SIZE BL_APP_HEADER_START + 0x0000
#define BL_APP_HEADER_CRC BL_APP_HEADER_START + 0x0004
#define BL_APP_HEADER_VALID BL_APP_HEADER_START + 0x0FFF

// Update header layout
#define BL_UPDATE_HEADER_UPDATE_SIZE BL_UPDATE_HEADER_START + 0x0000
#define BL_UPDATE_HEADER_CRC BL_UPDATE_HEADER_START + 0x0004
#define BL_UPDATE_HEADER_STATUS BL_UPDATE_HEADER_START + 0x0400
#define BL_UPDATE_HEADER_VALID BL_UPDATE_HEADER_START + 0x0FFF

// Pointer to no-cache no-alloc section of flash. We don't want to waste cache
// on this because whenever we want to read it, we'll want the latest version.
// Made const so it errors if we try to write (which will fail at runtime).
static volatile const uint8_t *const flash_read =
    (uint8_t *)FLASH_ADDR_NOCACHE_NOALLOC;

/**
 * @brief Get the application valid state.
 *
 * Result will be 0-8.
 *
 * @return uint8_t
 */
static inline uint8_t get_application_valid() {
    uint8_t valid = *(flash_read + BL_APP_HEADER_VALID);

    if (valid == 0)
        return 8;
    else
        return __builtin_ctz(valid);
}

/**
 * @brief Set the application valid byte to a given state.
 *
 * State is how many bits of valid byte will be 0.
 *
 * @param state State of valid byte. Can be 0-8.
 * @return int
 */
static inline int set_application_valid(uint8_t state) {
    if (state > 8) {
        state = 8;
    }

    uint8_t buf[256];
    memset(buf, 0xFF, 256);

    // Set state bits low
    uint8_t valid = ~((1 << state) - 1);
    buf[255] &= valid;

    flash_range_program((BL_APP_HEADER_VALID) & ~(0xFFu), buf, 256);

    if (*(flash_read + BL_APP_HEADER_VALID) != valid) {
        return -1;
    }

    return 0;
}

/**
 * @brief Get the size of the update from the update header.
 *
 * @return uint32_t
 */
static inline uint32_t get_update_size() {
    uint32_t size = *((uint32_t *)(flash_read + BL_UPDATE_HEADER_UPDATE_SIZE));

    if (size == UINT32_MAX) {
        return 0;
    } else {
        return size;
    }
}

/**
 * @brief Get the update crc from the update header.
 *
 * @return uint32_t
 */
static inline uint32_t get_update_crc() {
    return *((uint32_t *)(flash_read + BL_UPDATE_HEADER_CRC));
}

/**
 * @brief Get the size of the application from the app header.
 *
 * @return uint32_t
 */
static inline uint32_t get_application_size() {
    uint32_t size = *((uint32_t *)(flash_read + BL_APP_HEADER_SIZE));

    if (size == UINT32_MAX) {
        return 0;
    } else {
        return size;
    }
}

/**
 * @brief Get the application crc from the app header.
 *
 * @return uint32_t
 */
static inline uint32_t get_application_crc() {
    return *((uint32_t *)(flash_read + BL_APP_HEADER_APP_SIZE));
}

/**
 * @brief Write a 128 byte chunk of data to flash.
 *
 * Returns 0 if programming was successful, 1 if flash data doesn't match and
 * 2 if address is out of bounds.
 *
 * @param start Start of app section or update section. Must be a constant.
 * @param offset Offset within section with bottom 7 bits truncated (chunk
 * aligned).
 * @param buf Buffer containing 128 bytes of data.
 * @return int
 */
static inline int write_chunk(const uint32_t start, uint16_t offset,
                              uint8_t *data) {
    uint8_t buf[256];

    uint8_t addr_half_page = offset & 0x1;  // Get position within page

    // Physical flash address of start of page
    uint32_t addr_flash = start + (offset << 7);
    addr_flash &= 0xFFFF00;

    if ((addr_flash < start) | (addr_flash >= start + BL_UPDATE_SIZE)) {
        return 2;
    }

    // Fill half of page with received chunk
    memcpy(buf + addr_half_page * UPDATER_CHUNK_SIZE, data, UPDATER_CHUNK_SIZE);

    // Fill other half with 1s so it doesn't overwrite existing data
    memset(buf + (addr_half_page ? 0 : 1) * UPDATER_CHUNK_SIZE, 0xFF,
           UPDATER_CHUNK_SIZE);

    flash_range_program(addr_flash, buf, 256);

    // Read back data from flash and compare to packet
    int match = 0;
    volatile const uint8_t *actual =
        flash_read + addr_flash + addr_half_page * UPDATER_CHUNK_SIZE;

    // TODO: do one word at a time instead of one byte, will need to deal with
    // unaligned array from packet struct
    for (uint8_t i = 0; i < UPDATER_CHUNK_SIZE / sizeof(actual[0]); i++) {
        // If any bit is different, match won't be 0
        match |= actual[i] ^ data[i];
    }

    return (match == 0) ? 0 : 1;
}
