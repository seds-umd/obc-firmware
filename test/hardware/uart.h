#pragma once

#include "macros.h"
#include "misc.h"
#include <stdbool.h>

#define uart0 0
#define uart1 1
#define UART_PARITY_NONE 0
#define UART_UARTIFLS_RXIFLSEL_BITS 0

#define UART_DR(uart) uart_sim_get(uart)

typedef uint uart_inst_t;
typedef struct {
    volatile uint32_t ifls;
    volatile uint32_t dr;
} uart_hw_t;

////////// Simulation control functions //////////

// Initialize UART simulation
void uart_sim_init(uart_inst_t uart, uint buf_size);

// Deinitialize UART simulation
void uart_sim_deinit();

// Send data to simulation
void uart_sim_send(uint8_t *buf, uint len);

// Number of bytes in simulated UART FIFO
uint uart_sim_rx_buf_size();

// Get UART byte
uint8_t uart_sim_get(uart_inst_t uart);

// Send UART byte
void uart_putc_raw(uart_inst_t *uart, char c);

////////// Simulated sdk functions //////////

// Returns true if data in buffer
bool uart_is_readable(uart_inst_t uart);

// Returns a fake uart register struct with simulated data
uart_hw_t *uart_get_hw(uart_inst_t uart);

////////// Empty functions //////////

static inline uint uart_init(uart_inst_t uart, uint baudrate) {
    UNUSED(uart);

    return baudrate;
}

static inline void uart_set_hw_flow(uart_inst_t uart, bool cts, bool rts) {
    UNUSED(uart);
    UNUSED(cts);
    UNUSED(rts);
}

static inline void uart_set_format(uart_inst_t uart, uint data_bits,
                                   uint stop_bits, uint parity) {
    UNUSED(uart);
    UNUSED(data_bits);
    UNUSED(stop_bits);
    UNUSED(parity);
}

static inline void uart_set_fifo_enabled(uart_inst_t uart, bool enabled) {
    UNUSED(uart);
    UNUSED(enabled);
}

static inline void uart_set_irq_enables(uart_inst_t uart, bool rx_has_data,
                                        bool tx_needs_data) {
    UNUSED(uart);
    UNUSED(rx_has_data);
    UNUSED(tx_needs_data);
}
