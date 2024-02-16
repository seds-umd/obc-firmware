#pragma once

#include <stdint.h>

#include "macros.h"

typedef unsigned int uint;

static inline void hw_write_masked(volatile uint32_t *addr, uint32_t values,
                                   uint32_t write_mask) {
    UNUSED(addr);
    UNUSED(values);
    UNUSED(write_mask);
}
