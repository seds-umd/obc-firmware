// Adapted from real pico/util/queue.h

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "unity.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *data;
    uint16_t wptr;
    uint16_t rptr;
    uint16_t element_size;
    uint16_t element_count;
} queue_t;

static inline void *_queue_internal_element_ptr(queue_t *q, int index) {
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(q->element_count, index, "_queue_internal_element_ptr");
    return q->data + index * q->element_size;
}

static inline uint16_t queue_get_level(queue_t *q) {
    int32_t rc = (int32_t)q->wptr - (int32_t)q->rptr;
    if (rc < 0) {
        rc += q->element_count + 1;
    }

    return (uint16_t) rc;
}

static inline bool queue_is_empty(queue_t *q) {
    return queue_get_level(q) == 0;
}

static inline bool queue_is_full(queue_t *q) {
    return queue_get_level(q) == q->element_count;
}

static inline void queue_init(queue_t *q, int element_size, int element_count) {
    q->data = (uint8_t *) calloc(element_count + 1, element_size);
    q->element_count = (uint16_t)element_count;
    q->element_size = (uint16_t)element_size;
    q->wptr = 0;
    q->rptr = 0;
}

static inline void queue_free(queue_t *q) {
    free(q->data);
}

static inline void queue_add_blocking(queue_t *q, void *data) {
    TEST_ASSERT_FALSE_MESSAGE(queue_is_full(q), "queue_add_blocking");

    memcpy(_queue_internal_element_ptr(q, q->wptr), data, q->element_size);
    q->wptr++;
    
    if (q->wptr > q->element_count) {
        q->wptr = 0;
    }
}

static inline void queue_remove_blocking(queue_t *q, void *data) {
    TEST_ASSERT_FALSE_MESSAGE(queue_is_empty(q), "queue_remove_blocking");

    memcpy(data, _queue_internal_element_ptr(q, q->rptr), q->element_size);
    q->rptr++;

    if (q->rptr > q->element_count) {
        q->rptr = 0;
    }
}

static inline void queue_peek_blocking(queue_t *q, void *data) {
    TEST_ASSERT_FALSE_MESSAGE(queue_is_empty(q), "queue_peek_blocking");

    memcpy(data, _queue_internal_element_ptr(q, q->rptr), q->element_size);
}
