#include "unity.h"

#include "pico/time.h"
#include "scheduler.h"

static int count1, count2, count3;

void task1() { count1++; }

void task2() { count2++; }

void task3() { count3++; }

void test_scheduler() {
    // Setup
    count1 = 0;
    count2 = 0;
    count3 = 0;

    _pico_time_us = 0;

    TEST_ASSERT_EQUAL_INT(0, scheduler_init());

    // Tasks should be prioritized in order of being added
    TEST_ASSERT_EQUAL_INT(0, scheduler_add_task(task1, 200));
    TEST_ASSERT_EQUAL_INT(0, scheduler_add_task(task2, 500));
    TEST_ASSERT_EQUAL_INT(0, scheduler_add_task(task3, 1000));
    TEST_ASSERT_EQUAL_INT(1, scheduler_add_task(task3, 1000));

    // Test

    // Nothing should run initially
    TEST_ASSERT_EQUAL_INT(-1, scheduler_run());
    TEST_ASSERT_EQUAL_INT(0, count1);
    TEST_ASSERT_EQUAL_INT(0, count2);
    TEST_ASSERT_EQUAL_INT(0, count3);

    _pico_time_us = 200;

    // Only task1 should run
    TEST_ASSERT_EQUAL_INT(0, scheduler_run());
    TEST_ASSERT_EQUAL_INT(-1, scheduler_run());
    TEST_ASSERT_EQUAL_INT(1, count1);
    TEST_ASSERT_EQUAL_INT(0, count2);
    TEST_ASSERT_EQUAL_INT(0, count3);

    _pico_time_us = 500;

    // task1 and task2 should run
    TEST_ASSERT_EQUAL_INT(0, scheduler_run());
    TEST_ASSERT_EQUAL_INT(1, scheduler_run());
    TEST_ASSERT_EQUAL_INT(-1, scheduler_run());
    TEST_ASSERT_EQUAL_INT(2, count1);
    TEST_ASSERT_EQUAL_INT(1, count2);
    TEST_ASSERT_EQUAL_INT(0, count3);

    _pico_time_us = 1000;

    // Everything should run
    TEST_ASSERT_EQUAL_INT(0, scheduler_run());
    TEST_ASSERT_EQUAL_INT(1, scheduler_run());
    TEST_ASSERT_EQUAL_INT(2, scheduler_run());
    TEST_ASSERT_EQUAL_INT(-1, scheduler_run());
    TEST_ASSERT_EQUAL_INT(3, count1);
    TEST_ASSERT_EQUAL_INT(2, count2);
    TEST_ASSERT_EQUAL_INT(1, count3);
}
