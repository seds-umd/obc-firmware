#include "unity.h"
#include "tests.h"

#include <stdlib.h>
#include <time.h>

// Measure test run time - https://stackoverflow.com/a/76029051
#define CPUTIME(FCALL)                             \
    ({                                             \
        float START = clock();                     \
        FCALL;                                     \
        ((float)clock() - START) / CLOCKS_PER_SEC; \
    })

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
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_uart)));
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_queue)));
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_flash)));

    // Unit tests for actual firmware
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_commands)));
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_scheduler)));
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_openlst)));
    printf("Took %f sec\n", CPUTIME(RUN_TEST(test_updater)));

    return UNITY_END();
}
