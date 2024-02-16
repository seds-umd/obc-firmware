#pragma once

#include "hardware/irq.h"
#include "hardware/uart.h"

// WARNING: current pinouts are for debugging, must change for actual OBC hardware

////////// UART //////////

#define OPENLST_UART_ID uart0
#define OPENLST_UART_IRQ UART0_IRQ
#define OPENLST_UART_BAUD 115200
// #define OPENLST_UART_TX 12
// #define OPENLST_UART_RX 13
#define OPENLST_UART_TX 16
#define OPENLST_UART_RX 17
#define OPENLST_UART_CTS 14
#define OPENLST_UART_RTS 15

// TODO: enable flow control eventually
#define OPENLST_FLOW false

// Buffer size must be a power of 2
#define OPENLST_BUF_LEN 1024

////////// PINOUTS //////////

#define DEBUG_PIN 19

////////// SETTINGS //////////

// Size of scheduler task table
// The only time this should ever be defined elsewhere is in unit tests
#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS 20
#endif
