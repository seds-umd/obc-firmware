#pragma once

#include <stdint.h>

/**
 * @brief Calculate the CRC32 of a buffer.
 *
 * Code from pico-examples/dma/sniff_crc/sniff_crc.c.
 *
 * See https://github.com/raspberrypi/pico-feedback/issues/247 for settings.
 *
 * @param buf Input data
 * @param size Number of bytes to calculate CRC over
 * @return uint32_t
 */
uint32_t calc_crc32(const volatile uint8_t *const buf, uint32_t size);
