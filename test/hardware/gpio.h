#pragma once

#include "misc.h"
#include "macros.h"

#define GPIO_FUNC_UART 0

static inline void gpio_set_function(uint gpio, int fn) {
    UNUSED(gpio);
    UNUSED(fn);
}
