#include "unity.h"

#include "commands.h"
#include "macros.h"

static int count1, count2;

int command1(uint8_t *buf, int len) {
    UNUSED(len);

    count1++;

    return (int8_t)buf[1];
}

int command2(uint8_t *buf, int len) {
    UNUSED(len);

    count2++;

    return (int8_t)buf[1];
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

    uint8_t buf[] = {0x00, 123};

    // Test running commands
    TEST_ASSERT_EQUAL_INT(1, command_process(NULL, 1));
    TEST_ASSERT_EQUAL_INT(1, command_process(buf, 0));
    TEST_ASSERT_EQUAL_INT(2, command_process(buf, 2));

    TEST_ASSERT_EQUAL_INT(0, count1);
    TEST_ASSERT_EQUAL_INT(0, count2);

    buf[0] = 0x01;
    TEST_ASSERT_EQUAL_INT(123, command_process(buf, 2));
    TEST_ASSERT_EQUAL_INT(1, count1);
    TEST_ASSERT_EQUAL_INT(0, count2);

    buf[0] = 0x02;
    TEST_ASSERT_EQUAL_INT(123, command_process(buf, 2));
    TEST_ASSERT_EQUAL_INT(123, command_process(buf, 2));
    TEST_ASSERT_EQUAL_INT(123, command_process(buf, 2));
    TEST_ASSERT_EQUAL_INT(1, count1);
    TEST_ASSERT_EQUAL_INT(3, count2);

    // command_debug_print_opcodes();
}
