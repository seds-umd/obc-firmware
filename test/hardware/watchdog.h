#pragma once

#include "macros.h"

#include <stdint.h>

static inline void watchdog_reboot(uint32_t pc, uint32_t sp, uint32_t delay_ms) {
    UNUSED(pc);
    UNUSED(sp);
    UNUSED(delay_ms);
}
