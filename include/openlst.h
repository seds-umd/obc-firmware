#pragma once

#include <stdint.h>

void openlst_init();

void openlst_uart_isr();

void openlst_process();

void openlst_handle_packet(uint8_t *buf, uint8_t len);
