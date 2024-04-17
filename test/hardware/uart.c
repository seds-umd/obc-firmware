#include "uart.h"

#include "misc.h"

#include "unity.h"

#include <stdlib.h>

uart_inst_t *uart0;
uart_inst_t *uart1;

////////// Simulation control functions //////////

void uart_sim_init(uart_inst_t **uart, uint size) {
    *uart = malloc(sizeof(uart_inst_t));

    TEST_ASSERT_NOT_NULL(*uart);

    (*uart)->buf_size = size;
    (*uart)->rx_buf = malloc((*uart)->buf_size);
    (*uart)->tx_buf = malloc((*uart)->buf_size);

    TEST_ASSERT_NOT_NULL((*uart)->rx_buf);
    TEST_ASSERT_NOT_NULL((*uart)->tx_buf);

    (*uart)->rx_buf_rd = 0;
    (*uart)->rx_buf_wr = 0;

    (*uart)->tx_buf_rd = 0;
    (*uart)->tx_buf_wr = 0;
}

void uart_sim_deinit(uart_inst_t *uart) {
    free(uart->rx_buf);
    free(uart->tx_buf);
    free(uart);
}

void uart_sim_send(uart_inst_t *uart, uint8_t *buf, uint len) {
    TEST_ASSERT_LESS_THAN_UINT_MESSAGE(
        uart->buf_size, len + uart_sim_rx_buf_size(uart), "RX buffer overflow");

    for (uint i = 0; i < len; i++) {
        uart->rx_buf[uart->rx_buf_wr++] = buf[i];
        uart->rx_buf_wr %= uart->buf_size;
    }
}

uint uart_sim_rx_buf_size(uart_inst_t *uart) {
    return (uart->rx_buf_wr - uart->rx_buf_rd) % uart->buf_size;
}

uint8_t uart_sim_rx_get(uart_inst_t *uart) {
    TEST_ASSERT_NOT_EQUAL_UINT_MESSAGE(uart_sim_rx_buf_size(uart), 0,
                                       "RX buffer underflow");

    uint8_t x = uart->rx_buf[uart->rx_buf_rd++];
    uart->rx_buf_rd %= uart->buf_size;

    return x;
}

uint uart_sim_tx_buf_size(uart_inst_t *uart) {
    return (uart->tx_buf_wr - uart->tx_buf_rd) % uart->buf_size;
}

uint8_t uart_sim_tx_get(uart_inst_t *uart) {
    TEST_ASSERT_NOT_EQUAL_UINT_MESSAGE(uart_sim_tx_buf_size(uart), 0,
                                       "TX buffer underflow");

    uint8_t x = uart->tx_buf[uart->tx_buf_rd++];
    uart->tx_buf_rd %= uart->buf_size;

    return x;
}

////////// Simulated sdk functions //////////

bool uart_is_readable(uart_inst_t *uart) {
    return uart_sim_rx_buf_size(uart) > 0;
}

uart_hw_t *uart_get_hw(uart_inst_t *uart) {
    UNUSED(uart);

    return NULL;
}

void uart_putc_raw(uart_inst_t *uart, char c) {
    TEST_ASSERT_LESS_THAN_UINT_MESSAGE(uart->buf_size, uart_sim_tx_buf_size(uart), "TX buffer overflow");

    uart->tx_buf[uart->tx_buf_wr++] = c;
}
