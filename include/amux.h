#pragma once

#include <stdint.h>

void amux_init();
void set_select(uint8_t select_number);
void select_all();
uint32_t read_adc();
void read_temp(int channel);
uint16_t read_and_convert(uint8_t amux_input);
uint16_t getOutputCurrent(uint32_t outputVoltage, int pinNo);
uint16_t readrp2040Temp();