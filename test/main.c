#include "unity.h"

// Add function declarations here for every test
void test_uart();
void test_commands();
void test_scheduler();
void test_openlst();

// Empty setup and teardown so compiler doesn't get mad
void setUp() {}

void tearDown() {}

int main(void) {
    UNITY_BEGIN();

    // Unit tests for test infrastructure
    RUN_TEST(test_uart);

    // Unit tests for actual firmware
    RUN_TEST(test_commands);
    RUN_TEST(test_scheduler);
    RUN_TEST(test_openlst);

    return UNITY_END();
}
