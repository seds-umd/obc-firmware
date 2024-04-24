#pragma once

#include "macros.h"

#include <stdint.h>

typedef struct {
    uint32_t ctrl;
    uint32_t load;
    uint32_t reason;
    uint32_t scratch[8];
    uint32_t tick;
} watchdog_hw_t;

watchdog_hw_t _watchdog_hw;
watchdog_hw_t *watchdog_hw = &_watchdog_hw;

static inline void watchdog_reboot(uint32_t pc, uint32_t sp,
                                   uint32_t delay_ms) {
    UNUSED(pc);
    UNUSED(sp);
    UNUSED(delay_ms);
}
