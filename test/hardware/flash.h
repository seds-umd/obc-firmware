#pragma once

#include <stdint.h>
#include <stdlib.h>

#define PICO_FLASH_SIZE_BYTES 16*1024*1024

extern uint8_t sim_flash_buf[];

void flash_range_erase(uint32_t flash_offs, size_t count);
void flash_range_program(uint32_t flash_offs, const uint8_t *data, size_t count);

uint8_t sim_flash_read8(uint32_t addr);
uint16_t sim_flash_read16(uint32_t addr);
uint32_t sim_flash_read32(uint32_t addr);
