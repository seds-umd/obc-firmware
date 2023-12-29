#pragma once

#include <stdint.h>

// Mock definitions of pico/time.h functions for testing
// Mostly copied from original:
// https://github.com/raspberrypi/pico-sdk/blob/master/src/common/pico_time/include/pico/time.h

typedef uint64_t absolute_time_t;

extern absolute_time_t _pico_time_us;

static inline absolute_time_t get_absolute_time() {
    return _pico_time_us;
}

static inline absolute_time_t delayed_by_us(const absolute_time_t t, uint64_t us) {
    return t + us;
}

static inline int64_t absolute_time_diff_us(absolute_time_t from, absolute_time_t to) {
    return (int64_t)(to - from);
}
