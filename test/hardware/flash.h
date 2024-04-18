#pragma once

#include <stdint.h>
#include <stdlib.h>

#define PICO_FLASH_SIZE_BYTES 16*1024*1024

extern uint8_t sim_flash_buf[];

void flash_range_erase(uint32_t flash_offs, size_t count);
void flash_range_program(uint32_t flash_offs, const uint8_t *data, size_t count);
