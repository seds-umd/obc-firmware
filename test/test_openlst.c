#include "unity.h"

#include "command_handler.h"
#include "command_formats.h"
#include "openlst.h"
#include "hardware/uart.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 100k runs should be enough, and time based seeds means even very rare
// failures should be caught eventually since this runs after every commit
#define TEST_RUNS 100000

static int sum1, sum2, count1, count2;

static int command1(packet_t *pkt) {
    for (uint i=1; i<(pkt->lst_pkt->len - OPENLST_HEADER_SIZE); i++) {
        sum1 += pkt->lst_pkt->pld.buf[i];
        count1++;
    }

    return 0;
}

static int command2(packet_t *pkt) {
    for (uint i=1; i<(pkt->lst_pkt->len - OPENLST_HEADER_SIZE); i++) {
        sum2 += pkt->lst_pkt->pld.buf[i];
        count2++;
    }

    return 0;
}

static uint8_t *make_packet(uint16_t hwid, uint16_t seq, uint8_t sys, uint8_t cmd, uint8_t *data, int data_len) {
    int full_len = data_len + 9;
    int packet_len = data_len + 6;

    uint8_t *pkt = malloc(full_len);

    pkt[0] = 0x22;
    pkt[1] = 0x69;
    pkt[2] = packet_len;
    pkt[3] = hwid & 0xFF;
    pkt[4] = (hwid >> 8) & 0xFF;
    pkt[5] = seq & 0xFF;
    pkt[6] = (seq >> 8) & 0xFF;
    pkt[7] = sys;
    pkt[8] = cmd;

    memcpy(pkt + 9, data, data_len);

    return pkt;
}

void test_openlst() {
    char str_buf[100];

    sum1 = 0;
    sum2 = 0;
    count1 = 0;
    count2 = 0;
    int sum1_ref = 0;
    int sum2_ref = 0;
    int count1_ref = 0;
    int count2_ref = 0;

    uart_sim_init(&uart0, 2048);

    command_init();
    command_register(0x00, command1);
    command_register(0x01, command2);

    openlst_init();

    for (int i=0; i<TEST_RUNS; i++) {
        int pld_len = rand() % 242;
        pld_len = (pld_len == 0) ? 1 : pld_len;

        uint8_t *pld = malloc(pld_len);
        pld[0] = i % 2;

        for (int j=1; j<pld_len; j++) {
            uint8_t x = rand() & 0xFF;
            pld[j] = x;

            if (i % 2 == 0) {
                sum1_ref += x;
                count1_ref++;
            } else {
                sum2_ref += x;
                count2_ref++;
            }
        }

        uint8_t *pkt = make_packet(0x1234, i, 0x01, 0x11, pld, pld_len);

        uart_sim_send(uart0, pkt, pld_len + 9);

        // Feed all bytes in
        while (uart_sim_rx_buf_size(uart0) > 0) {
            openlst_uart_isr();
        }

        openlst_process();

        free(pkt);
        free(pld);

        snprintf(str_buf, 100, "Iteration %d", i);

        TEST_ASSERT_EQUAL_INT_MESSAGE(count1_ref, count1, str_buf);
        TEST_ASSERT_EQUAL_INT_MESSAGE(count2_ref, count2, str_buf);
        TEST_ASSERT_EQUAL_INT_MESSAGE(sum1_ref, sum1, str_buf);
        TEST_ASSERT_EQUAL_INT_MESSAGE(sum2_ref, sum2, str_buf);
    }

    openlst_deinit();
    uart_sim_deinit(uart0);
}
