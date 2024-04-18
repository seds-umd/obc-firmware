#pragma once

#include <stdint.h>

/**
 * @brief Calculate the CRC32 of a buffer.
 *
 * Code from pico-examples/dma/sniff_crc/sniff_crc.c.
 *
 * See https://github.com/raspberrypi/pico-feedback/issues/247 for settings.
 *
 * @param buf Input data, must be word aligned
 * @param size Number of words to calculate CRC over
 * @return uint32_t
 */
uint32_t calc_crc32(uint32_t *buf, uint32_t size);
