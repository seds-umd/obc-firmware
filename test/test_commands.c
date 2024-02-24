#include "unity.h"

#include "command_formats.h"
#include "command_handler.h"
#include "macros.h"

static int count1, count2;

int command1(packet_t *pkt) {
    count1++;

    return pkt->lst_pkt->pld.buf[1];
}

int command2(packet_t *pkt) {
    count2++;

    return pkt->lst_pkt->pld.buf[1];
}

void test_commands() {
    count1 = 0;
    count2 = 0;

    command_init();

    // Register commands and make sure they can only be registered once
    TEST_ASSERT_EQUAL_INT(0, command_register(0x01, command1));
    TEST_ASSERT_EQUAL_INT(2, command_register(0x01, command1));
    TEST_ASSERT_EQUAL_INT(0, command_register(0x02, command2));
    TEST_ASSERT_EQUAL_INT(2, command_register(0x02, command2));

    openlst_packet_t lst_pkt;
    lst_pkt.pld.buf[0] = 0x00;
    lst_pkt.pld.buf[1] = 0xAB;
    lst_pkt.len = 2 + OPENLST_HEADER_SIZE;

    packet_t pkt;
    pkt.lst_pkt = &lst_pkt;
    pkt.type = PACKET_TYPE_OPENLST;

    // Test running commands
    TEST_ASSERT_EQUAL_INT(1, command_process(NULL));
    TEST_ASSERT_EQUAL_INT(2, command_process(&pkt));

    // Nothing should have run yet because opcode was zero
    TEST_ASSERT_EQUAL_INT(0, count1);
    TEST_ASSERT_EQUAL_INT(0, count2);

    // Run command1 once
    lst_pkt.pld.buf[0] = 0x01;
    TEST_ASSERT_EQUAL_HEX(0xAB, command_process(&pkt));
    TEST_ASSERT_EQUAL_INT(1, count1);
    TEST_ASSERT_EQUAL_INT(0, count2);

    // Run command2 3 times
    lst_pkt.pld.buf[0] = 0x02;
    TEST_ASSERT_EQUAL_HEX(0xAB, command_process(&pkt));
    TEST_ASSERT_EQUAL_HEX(0xAB, command_process(&pkt));
    TEST_ASSERT_EQUAL_HEX(0xAB, command_process(&pkt));
    TEST_ASSERT_EQUAL_INT(1, count1);
    TEST_ASSERT_EQUAL_INT(3, count2);

    // command_debug_print_opcodes();
}
