#include "openlst.h"

#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"
#include "string.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "config.h"
#include "commands.h"
#include "command_formats.h"
#include "macros.h"

static uint8_t rx_buf[OPENLST_BUF_LEN];
static uint8_t pkt_buf[OPENLST_MAX_PAYLOAD];
static volatile uint16_t rx_buf_wr;
static uint16_t rx_buf_rd;

static uint16_t buffer_len() {
    return ((uint16_t) (rx_buf_wr - rx_buf_rd)) % OPENLST_BUF_LEN;
}

void openlst_init() {
    uart_init(OPENLST_UART_ID, OPENLST_UART_BAUD);

    // Configure pins
    gpio_set_function(OPENLST_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(OPENLST_UART_RX, GPIO_FUNC_UART);
    gpio_set_function(OPENLST_UART_CTS, GPIO_FUNC_UART);
    gpio_set_function(OPENLST_UART_RTS, GPIO_FUNC_UART);

    // Configure UART settings
    uart_set_hw_flow(OPENLST_UART_ID, OPENLST_FLOW, OPENLST_FLOW);
    uart_set_format(OPENLST_UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(OPENLST_UART_ID, true);

    // Configure interrupt
    irq_set_exclusive_handler(OPENLST_UART_IRQ, openlst_uart_isr);
    irq_set_enabled(OPENLST_UART_IRQ, true);
    uart_set_irq_enables(OPENLST_UART_ID, true, false);

    rx_buf_wr = 0;
    rx_buf_rd = 0;

    // Set interrupt to trigger when FIFO is 7/8 full
    hw_write_masked(&uart_get_hw(OPENLST_UART_ID)->ifls, 0b100,
                    UART_UARTIFLS_RXIFLSEL_BITS);
}

void openlst_uart_isr() {
    while (uart_is_readable(OPENLST_UART_ID)) {
        // Access register directly to speed things up
        rx_buf[rx_buf_wr++] = UART_DR(OPENLST_UART_ID);
        rx_buf_wr %= OPENLST_BUF_LEN;
    }
}

void openlst_process() {
    // TODO: test entire function, lots of places for off by one errors

    uint16_t buf_len = buffer_len();
    uint16_t start_idx = rx_buf_rd;

    // Loop until buffer is empty or only a partial packet remains
    while (1) {
        uint16_t consumed;

        // Loop until start bytes are found or we run out of bytes
        while (1) {
            consumed = ((uint16_t) (rx_buf_rd - start_idx)) % OPENLST_BUF_LEN;

            uint8_t byte1 = rx_buf[rx_buf_rd];
            uint8_t byte2 = rx_buf[(rx_buf_rd + 1) % OPENLST_BUF_LEN];

            // Less than 3 bytes left, leave them for next time
            if (consumed >= buf_len - 3) {
                return;
            }

            // Check for start bytes
            if ((byte1 == OPENLST_START_0) && (byte2 == OPENLST_START_1)) {
                break;
            }

            rx_buf_rd++;
            rx_buf_rd %= OPENLST_BUF_LEN;
        }

        // At this point, we should have at least the 2 start bytes and the
        // length byte in the buffer. The read pointer will stay at the start
        // of the packet until the entire packet is in the buffer and able to
        // be processed.

        uint8_t pkt_len = rx_buf[(rx_buf_rd + 2) % OPENLST_BUF_LEN];

        // If the entire packet isn't fully received yet, wait until next time
        if (buf_len < pkt_len + 3) {
            return;
        }

        // Packet starts 3 bytes after read pointer (ignoring start and length)
        uint8_t *pkt = rx_buf + (rx_buf_rd + 3) % OPENLST_BUF_LEN;

        // Use an intermediate buffer if the packet wraps around the end of
        // the buffer.
        if ((rx_buf_rd + 3) % OPENLST_BUF_LEN + pkt_len >= OPENLST_BUF_LEN) {
            int count = OPENLST_BUF_LEN - (rx_buf_rd + 3);
            memcpy(pkt_buf, rx_buf + rx_buf_rd + 3, count);
            memcpy(pkt_buf + count, rx_buf, pkt_len - count);

            pkt = pkt_buf;
        }

        openlst_handle_packet(pkt, pkt_len);

        // Move read pointer to after packet
        rx_buf_rd = (rx_buf_rd + pkt_len + 3) % OPENLST_BUF_LEN;
    }
}

void openlst_handle_packet(uint8_t *buf, uint8_t len) {
    openlst_packet_t *pkt = (openlst_packet_t *)buf;
    uint8_t data_len = len - sizeof(openlst_packet_header_t);

    switch (pkt->hdr.command) {
        case ASCII:
            // TODO: handle command return value
            command_process(pkt->pld.buf, data_len);
            break;

        // Ignore these commands
        case BOOTLOADER_PING:
        case BOOTLOADER_ACK: // TODO: handle ACK
        case BOOTLOADER_WRITE_PAGE:
        case BOOTLOADER_ERASE:
        case ACK: // TODO: handle ACK
        case NACK: // TODO: handle NACK
        case REBOOT:
        default:
            break;
    }
}
