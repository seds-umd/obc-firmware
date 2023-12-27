#include "unity.h"

// It's a bit hacky to include a .c file but it keeps things simple here
// TODO: find a better way to do this as unit tests get more complicated
#include "test_commands.c"

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_commands);

    return UNITY_END();
}
