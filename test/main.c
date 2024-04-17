#include "unity.h"
#include "tests.h"

#include <stdlib.h>
#include <time.h>

// Empty setup and teardown so compiler doesn't get mad
void setUp() {}

void tearDown() {}

int main(void) {
    uint32_t seed = time(NULL);
    srand(seed);

    // Log seed so failures can be repeated
    printf("Seed %d\n", seed);

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
