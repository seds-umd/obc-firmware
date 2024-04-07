#include "hardware/dma.h"
#include "pico/time.h"

#include "misc.h"

// This file contains definitions of global variables for mock hardware

// dma.h
dma_hw_t _dma_hw_internal;
dma_hw_t *dma_hw = &_dma_hw_internal;

// time.h
absolute_time_t _pico_time_us;
