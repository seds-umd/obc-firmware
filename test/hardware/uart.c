#include "uart.h"

#include "unity.h"
#include "misc.h"
// #include <assert.h>
#include <stdlib.h>

static uint8_t *rx_buf;
static uint8_t *tx_buf;
static uint buf_size;
static uint rx_buf_rd, rx_buf_wr;
static uint tx_buf_rd, tx_buf_wr;

////////// Simulation control functions //////////

void uart_sim_init(uart_inst_t uart, uint size) {
    UNUSED(uart); // TODO: implement both uarts

    buf_size = size;
    rx_buf = malloc(buf_size);
    tx_buf = malloc(buf_size); // TX not yet implemented

    TEST_ASSERT_NOT_NULL(rx_buf);
    TEST_ASSERT_NOT_NULL(tx_buf);

    rx_buf_rd = 0;
    rx_buf_wr = 0;

    tx_buf_rd = 0;
    tx_buf_wr = 0;
}

void uart_sim_deinit() {
    free(rx_buf);
    free(tx_buf);
}

void uart_sim_send(uint8_t *buf, uint len) {
    if (len > buf_size + uart_sim_rx_buf_size()) {
        TEST_FAIL_MESSAGE("Too many bytes written");
    }

    for (uint i=0; i<len; i++) {
        rx_buf[rx_buf_wr++] = buf[i];
        rx_buf_wr %= buf_size;
    }
}

uint uart_sim_rx_buf_size() {
    return (rx_buf_wr - rx_buf_rd) % buf_size;
}

uint8_t uart_sim_get(uart_inst_t uart) {
    if (uart == uart0) {
        if (uart_sim_rx_buf_size() == 0) {
            TEST_FAIL_MESSAGE("Attempted to read from empty buffer");
        }

        uint8_t x = rx_buf[rx_buf_rd++];
        rx_buf_rd %= buf_size;

        return x;
    } else {
        TEST_FAIL_MESSAGE("Only UART0 is supported right now");
    }
}

void uart_putc_raw(uart_inst_t *uart, char c) {
    UNUSED(uart);
    UNUSED(c);
}

////////// Simulated sdk functions //////////

bool uart_is_readable(uart_inst_t uart) {
    UNUSED(uart);

    return uart_sim_rx_buf_size() > 0;
}

uart_hw_t *uart_get_hw(uart_inst_t uart) {
    UNUSED(uart);

    return NULL;
}
