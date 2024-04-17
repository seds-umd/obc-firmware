#pragma once

#include "macros.h"
#include "misc.h"

#include <stdbool.h>

#define UART_PARITY_NONE 0
#define UART_UARTIFLS_RXIFLSEL_BITS 0

#define UART_DR(uart) uart_sim_rx_get(uart)

typedef struct {
    uint8_t *rx_buf;
    uint8_t *tx_buf;
    uint buf_size;
    uint rx_buf_rd, rx_buf_wr;
    uint tx_buf_rd, tx_buf_wr;
} uart_inst_t;

typedef struct {
    volatile uint32_t ifls;
    volatile uint32_t dr;
} uart_hw_t;

extern uart_inst_t *uart0;
extern uart_inst_t *uart1;

////////// Simulation control functions //////////

// Initialize UART simulation
void uart_sim_init(uart_inst_t **uart, uint buf_size);

// Deinitialize UART simulation
void uart_sim_deinit(uart_inst_t *uart);

// Send data to simulation
void uart_sim_send(uart_inst_t *uart, uint8_t *buf, uint len);

// Number of bytes in RX FIFO
uint uart_sim_rx_buf_size(uart_inst_t *uart);

// Get UART byte from RX FIFO
uint8_t uart_sim_rx_get(uart_inst_t *uart);

// Number of bytes in TX FIFO
uint uart_sim_tx_buf_size(uart_inst_t *uart);

// Get UART byte from TX FIFO
uint8_t uart_sim_tx_get(uart_inst_t *uart);

////////// Simulated sdk functions //////////

// Returns true if data in buffer
bool uart_is_readable(uart_inst_t *uart);

// Returns a fake uart register struct with simulated data
uart_hw_t *uart_get_hw(uart_inst_t *uart);

// Send UART byte
void uart_putc_raw(uart_inst_t *uart, char c);

////////// Empty functions //////////

static inline uint uart_init(uart_inst_t *uart, uint baudrate) {
    UNUSED(uart);

    return baudrate;
}

static inline void uart_set_hw_flow(uart_inst_t *uart, bool cts, bool rts) {
    UNUSED(uart);
    UNUSED(cts);
    UNUSED(rts);
}

static inline void uart_set_format(uart_inst_t *uart, uint data_bits,
                                   uint stop_bits, uint parity) {
    UNUSED(uart);
    UNUSED(data_bits);
    UNUSED(stop_bits);
    UNUSED(parity);
}

static inline void uart_set_fifo_enabled(uart_inst_t *uart, bool enabled) {
    UNUSED(uart);
    UNUSED(enabled);
}

static inline void uart_set_irq_enables(uart_inst_t *uart, bool rx_has_data,
                                        bool tx_needs_data) {
    UNUSED(uart);
    UNUSED(rx_has_data);
    UNUSED(tx_needs_data);
}
