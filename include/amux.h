#pragma once

#include <stdint.h>

void amux_init();
void set_select(uint8_t select_number);
void select_all();
uint32_t read_adc();
void read_temp(int channel);
float read_and_convert(uint8_t amux_input);
float getOutputCurrent(uint32_t outputVoltage, int pinNo);