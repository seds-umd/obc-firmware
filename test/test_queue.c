#include "unity.h"

#include "pico/util/queue.h"

void test_queue() {
    queue_t q;

    volatile int x1 = rand(), x2 = rand(), x3 = rand();

    queue_init(&q, sizeof(int), 3);

    TEST_ASSERT(queue_is_empty(&q));

    queue_add_blocking(&q, (void *) &x1);
    queue_add_blocking(&q, (void *) &x2);
    queue_add_blocking(&q, (void *) &x3);

    TEST_ASSERT_EQUAL_INT(3, queue_get_level(&q));
    TEST_ASSERT(queue_is_full(&q));

    int y1, y2, y3;

    queue_remove_blocking(&q, (void *) &y1);
    queue_remove_blocking(&q, (void *) &y2);
    queue_remove_blocking(&q, (void *) &y3);

    TEST_ASSERT_EQUAL(x1, y1);
    TEST_ASSERT_EQUAL(x2, y2);
    TEST_ASSERT_EQUAL(x3, y3);

    TEST_ASSERT_TRUE(queue_is_empty(&q));
    TEST_ASSERT_FALSE(queue_is_full(&q));

    queue_free(&q);
}
