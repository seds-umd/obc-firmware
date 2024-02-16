#include "unity.h"

#include "hardware/uart.h"
#include <stdlib.h>

#define TEST_RUNS 1000
#define BUF_SIZE 256

void test_uart() {
    // Make test deterministic
    srand(0x58209348);

    // Relatively small buffer to keep things simple
    uart_sim_init(uart0, BUF_SIZE);
    uint8_t buf[BUF_SIZE];

    // Repeat test a bunch of times
    for (int i=0; i<TEST_RUNS; i++) {
        int count = rand() % BUF_SIZE;

        if (count == 0) {
            count = 1;
        }

        // Fill buffer with random data
        for (int j=0; j<count; j++) {
            buf[j] = (uint8_t) rand();
        }

        // Send data
        uart_sim_send(buf, count);

        // Receive and check data
        uint8_t rx_buf[BUF_SIZE];

        for (int j=0; j<count; j++) {
            rx_buf[j] = UART_DR(uart0);
        }

        TEST_ASSERT_EQUAL_UINT8_ARRAY(buf, rx_buf, count);
        TEST_ASSERT_EQUAL(0, uart_sim_rx_buf_size());
        TEST_ASSERT_EQUAL(false, uart_is_readable(uart0));
    }

    uart_sim_deinit();
}
