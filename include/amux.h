#pragma once

#include <stdint.h>

void amux_init();
void set_select(uint8_t select_number);
void select_all();
uint32_t read_adc();
void read_temp(int channel);