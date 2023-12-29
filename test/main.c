#include "unity.h"

// Add function declarations here for every test

void test_commands();

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_commands);

    return UNITY_END();
}
