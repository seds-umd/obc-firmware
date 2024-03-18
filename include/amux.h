#pragma once

#include <stdint.h>

void amux_init();
void set_select(uint8_t select_number);
void select_all();
uint16_t read_adc();