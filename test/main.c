#include "unity.h"
#include "tests.h"

// Empty setup and teardown so compiler doesn't get mad
void setUp() {}

void tearDown() {}

int main(void) {
    UNITY_BEGIN();

    TEST_MESSAGE(
        "Note: line numbers for failed tests correspond to the file the test "
        "is defined in, not main.c like the message says");

    // Unit tests for test infrastructure
    RUN_TEST(test_uart);
    RUN_TEST(test_queue);

    // Unit tests for actual firmware
    RUN_TEST(test_commands);
    RUN_TEST(test_scheduler);
    RUN_TEST(test_openlst);

    return UNITY_END();
}
