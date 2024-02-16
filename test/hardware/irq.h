#pragma once

#include "misc.h"
#include "macros.h"
#include <stdbool.h>

#define UART0_IRQ 0

static inline void irq_set_exclusive_handler(uint num, void (*handler)()) {
    UNUSED(num);
    UNUSED(handler);
}

static inline void irq_set_enabled(uint num, bool enabled) {
    UNUSED(num);
    UNUSED(enabled);
}
