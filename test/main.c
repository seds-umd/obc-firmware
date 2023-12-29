#include "unity.h"

// Add function declarations here for every test
void test_commands();
void test_scheduler();

// Empty setup and teardown so compiler doesn't get mad
void setUp() {
}

void tearDown() {
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_commands);
    RUN_TEST(test_scheduler);

    return UNITY_END();
}
